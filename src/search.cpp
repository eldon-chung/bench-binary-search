#include <immintrin.h>
#include <stdint.h>

#include <array>
#include <iostream>

#include "File.h"
#include "search.h"

// number of elements, not bytes that we are going to store
constexpr size_t chunk_size = 1024;

// basic iterative binary search
bool binary_search_basic(File const &file, uint32_t query) {
  ssize_t left_idx = 0;
  ssize_t right_idx = (ssize_t)file.size(32) - 1;

  while (left_idx <= right_idx) {
    ssize_t mid_idx = (right_idx - left_idx) / 2 + left_idx;

    if (file.read32_reintc((ssize_t)mid_idx) == query) {
      return true;
    }

    if (query < file.read32_reintc((ssize_t)mid_idx)) {
      right_idx = mid_idx - 1;
    } else {
      left_idx = mid_idx + 1;
    }
  }
  return false;
}

// branchless and cache sensitive binary search
// bool binary_search_basic(File const &file, uint32_t query) {
//   ssize_t left_idx = 0;
//   ssize_t right_idx = file.size(32) - 1;

//   while (left_idx <= right_idx) {
//     ssize_t mid_idx = (right_idx - left_idx) / 2 + left_idx;

//     if (file.read32_reintc(mid_idx) == query) {
//       return true;
//     }

//     if (query < file.read32_reintc(mid_idx)) {
//       right_idx = mid_idx - 1;
//     } else {
//       left_idx = mid_idx + 1;
//     }
//   }
//   return false;
// }

// basic linear search
bool linear_search_basic(File const &file, uint32_t query) {
  const uint32_t *begin = file.begin();
  const uint32_t *end = file.end();
  while (begin != end) {
    if (*begin == query) {
      return true;
    }
    ++begin;
  }
  return false;
}

// basic linear search with early termination
bool linear_search_basic_early_term(File const &file, uint32_t query) {
  auto begin = file.begin();
  auto end = file.end();
  while (begin != end) {
    if (*begin >= query) {
      return *begin == query;
    }
    ++begin;
  }
  return false;
}

// manually vectorised linear search
bool linear_search_vector(File const &file, uint32_t query) {
  // repeat query 8 times into a 256bit register
  auto broadcasted_register = _mm256_set1_epi32(query);
  for (size_t chunk_idx = 0; chunk_idx < (file.size(32) / chunk_size) + 1;
       ++chunk_idx) {

    if ((chunk_idx + 1) * chunk_size > file.size(32)) {
      // file is too small, we just do a basic for loop
      for (size_t idx = chunk_idx * chunk_size; idx < file.size(32); ++idx) {
        if (file.read32_reintc(idx) == query) {
          return true;
        }
      }
      return false;

    } else {
      // do loops based on chunk size
      // std::cout << "chunk loop" << std::endl;
      auto begin = (file.addr32(chunk_idx * chunk_size));
      auto end = (file.addr32((chunk_idx + 1) * chunk_size));

      while (begin != end) {
        // load 8 of them
        auto loaded_register =
            _mm256_loadu_ps(reinterpret_cast<const float *>(begin));

        // do a comparison
        auto cmp_res = _mm256_cmpeq_epi32(broadcasted_register,
                                          _mm256_castps_si256(loaded_register));

        // if any are yes, we break and output true
        int movemask = _mm256_movemask_ps(_mm256_castsi256_ps(cmp_res));
        if (movemask != 0) {
          // std::cerr << "normal chunk idx: " << chunk_idx << std::endl;
          return true;
        }

        // move up by 8 uint32_ts
        begin += 8;
      }
    }
  }
  return false;
}

// manually vectorised linear search
bool linear_search_vector_early_term(File const &file, uint32_t query) {
  // repeat query 8 times into a 256bit register
  auto broadcasted_register = _mm256_set1_epi32(query);
  for (size_t chunk_idx = 0; chunk_idx < (file.size(32) / chunk_size) + 1;
       ++chunk_idx) {

    if ((chunk_idx + 1) * chunk_size > file.size(32)) {
      // file is too small, we just do a basic for loop
      for (size_t idx = chunk_idx * chunk_size; idx < file.size(32); ++idx) {
        if (file.read32_reintc(idx) >= query) {
          // std::cerr << "small loop returning: " << std::endl;
          return file.read32_reintc(idx) == query;
        }
      }
      return false;

    } else {
      // do loops based on chunk size
      auto begin = (file.addr32(chunk_idx * chunk_size));
      auto end = (file.addr32((chunk_idx + 1) * chunk_size));

      while (begin != end) {
        // load 8 of them
        auto loaded_register =
            _mm256_loadu_ps(reinterpret_cast<const float *>(begin));

        // do a comparison
        auto cmp_res = _mm256_cmpeq_epi32(_mm256_castps_si256(loaded_register),
                                          broadcasted_register);

        // unsigned comparison the query against the loaded register
        auto max_register = _mm256_max_epu32(
            _mm256_castps_si256(loaded_register), broadcasted_register);
        auto max_cmp_res = _mm256_cmpeq_epi32(
            _mm256_castps_si256(loaded_register), max_register);

        // if any are yes, we break and output true
        int cmp_movemask = _mm256_movemask_ps(_mm256_castsi256_ps(cmp_res));
        if (cmp_movemask != 0) {
          return true;
        }

        // if any are yes, we break and output false; early termination
        int gt_movemask = _mm256_movemask_ps(_mm256_castsi256_ps(max_cmp_res));
        if (gt_movemask != 0) {
          return false;
        }

        // move up by 8 uint32_t
        begin += 8;
      }
    }
  }
  return false;
}

bool linear_search_vector_early_term_twin_load(File const &file,
                                               uint32_t query) {
  // repeat query 8 times into a 256bit register
  auto broadcasted_register = _mm256_set1_epi32(query);
  for (size_t chunk_idx = 0; chunk_idx < (file.size(32) / chunk_size) + 1;
       ++chunk_idx) {

    if ((chunk_idx + 1) * chunk_size > file.size(32)) {
      // file is too small, we just do a basic for loop
      for (size_t idx = chunk_idx * chunk_size; idx < file.size(32); ++idx) {
        if (file.read32_reintc(idx) >= query) {
          // std::cerr << "small loop returning: " << std::endl;
          return file.read32_reintc(idx) == query;
        }
      }
      return false;

    } else {
      // do loops based on chunk size
      // std::cout << "chunk loop" << std::endl;
      auto begin = (file.addr32(chunk_idx * chunk_size));
      auto end = (file.addr32((chunk_idx + 1) * chunk_size));

      while (begin != end) {
        // load 16 of them
        auto loaded_register1 =
            _mm256_loadu_ps(reinterpret_cast<const float *>(begin));
        auto loaded_register2 =
            _mm256_loadu_ps(reinterpret_cast<const float *>(begin + 8));

        // do a comparison
        auto cmp_res1 = _mm256_cmpeq_epi32(
            _mm256_castps_si256(loaded_register1), broadcasted_register);
        auto cmp_res2 = _mm256_cmpeq_epi32(
            _mm256_castps_si256(loaded_register2), broadcasted_register);

        auto max_register2 = _mm256_max_epu32(
            _mm256_castps_si256(loaded_register2), broadcasted_register);
        auto max_cmp_res2 = _mm256_cmpeq_epi32(
            _mm256_castps_si256(loaded_register2), max_register2);

        // if any are yes, we break and output true
        int cmp_movemask1 = _mm256_movemask_ps(_mm256_castsi256_ps(cmp_res1));
        int cmp_movemask2 = _mm256_movemask_ps(_mm256_castsi256_ps(cmp_res2));
        if (cmp_movemask1 != 0 || cmp_movemask2 != 0) {
          return true;
        }

        int gt_movemask2 =
            _mm256_movemask_ps(_mm256_castsi256_ps(max_cmp_res2));
        // tbh we can just check the second one because the list is sorted
        if (gt_movemask2 != 0) {
          return false;
        }

        // move up by 8 uint32_t
        begin += 16;
      }
    }
  }
  return false;
}

bool linear_search_vector_twin_load(File const &file, uint32_t query) {
  // repeat query 8 times into a 256bit register
  auto broadcasted_register = _mm256_set1_epi32(query);
  for (size_t chunk_idx = 0; chunk_idx < (file.size(32) / chunk_size) + 1;
       ++chunk_idx) {

    if ((chunk_idx + 1) * chunk_size > file.size(32)) {
      // file is too small, we just do a basic for loop
      for (size_t idx = chunk_idx * chunk_size; idx < file.size(32); ++idx) {
        if (file.read32_reintc(idx) >= query) {
          // std::cerr << "small loop returning: " << std::endl;
          return file.read32_reintc(idx) == query;
        }
      }
      return false;

    } else {
      // do loops based on chunk size
      // std::cout << "chunk loop" << std::endl;
      auto begin = (file.addr32(chunk_idx * chunk_size));
      auto end = (file.addr32((chunk_idx + 1) * chunk_size));

      while (begin != end) {
        // load 16 of them
        auto loaded_register1 =
            _mm256_loadu_ps(reinterpret_cast<const float *>(begin));
        auto loaded_register2 =
            _mm256_loadu_ps(reinterpret_cast<const float *>(begin + 8));

        // do a comparison
        auto cmp_res1 = _mm256_cmpeq_epi32(
            _mm256_castps_si256(loaded_register1), broadcasted_register);
        auto cmp_res2 = _mm256_cmpeq_epi32(
            _mm256_castps_si256(loaded_register2), broadcasted_register);

        // unsigned comparison the query against the loaded register
        // auto max_register1 = _mm256_max_epu32(
        //     _mm256_castps_si256(loaded_register1), broadcasted_register);
        // auto max_cmp_res1 = _mm256_cmpeq_epi32(
        //     _mm256_castps_si256(loaded_register1), max_register1);

        // auto max_register2 = _mm256_max_epu32(
        //     _mm256_castps_si256(loaded_register2), broadcasted_register);
        // auto max_cmp_res2 = _mm256_cmpeq_epi32(
        //     _mm256_castps_si256(loaded_register2), max_register2);

        // if any are yes, we break and output true
        int cmp_movemask1 = _mm256_movemask_ps(_mm256_castsi256_ps(cmp_res1));
        int cmp_movemask2 = _mm256_movemask_ps(_mm256_castsi256_ps(cmp_res2));
        if (cmp_movemask1 != 0 || cmp_movemask2 != 0) {
          // std::cerr << "cmp_movemask: " << cmp_movemask << std::endl;
          // std::cerr << "early term chunk idx: " << chunk_idx << std::endl;
          return true;
        }

        // move up by 8 uint32_t
        begin += 16;
      }
    }
  }
  return false;
}
