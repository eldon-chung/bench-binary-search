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
  for (size_t index = 0; index < file.size(32); ++index) {
    if (file.read32_reintc(index) == query) {
      return true;
    }
  }
  return false;
}

// basic linear search with early termination
bool linear_search_basic_early_term(File const &file, uint32_t query) {
  // std::cerr << "query:" << query << std::endl;
  // std::cerr << file.size(32) << std::endl;

  for (size_t index = 0; index < file.size(32); ++index) {
    if (file.read32_reintc(index) >= query) {
      // std::cerr << index << std::endl;
      return file.read32_reintc(index) == query;
    }
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
      const float *begin =
          reinterpret_cast<const float *>(file.addr32(chunk_idx * chunk_size));
      const float *end = reinterpret_cast<const float *>(
          file.addr32((chunk_idx + 1) * chunk_size));

      while (begin != end) {
        // load 8 of them
        auto loaded_register = _mm256_loadu_ps(begin);

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
      // std::cout << "chunk loop" << std::endl;
      const float *begin =
          reinterpret_cast<const float *>(file.addr32(chunk_idx * chunk_size));
      const float *end = begin + chunk_size;

      while (begin != end) {
        // load 8 of them
        auto loaded_register = _mm256_loadu_ps(begin);

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
          // std::cerr << "cmp_movemask: " << cmp_movemask << std::endl;
          // std::cerr << "early term chunk idx: " << chunk_idx << std::endl;
          return true;
        }

        // if any are yes, we break and output false; early termination
        int gt_movemask = _mm256_movemask_ps(_mm256_castsi256_ps(max_cmp_res));
        if (gt_movemask != 0) {
          // std::cerr << "gt_movemask: " << gt_movemask << std::endl;
          // std::cerr << "early term chunk idx: " << chunk_idx << std::endl;
          return false;
        }

        // move up by 8 uint32_t
        begin += 8;
      }
    }
  }
  return false;
}
