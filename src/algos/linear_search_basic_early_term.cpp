#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <string_view>
#include <vector>

#include "../File.h"
#include "../search.h"

void test_all(File const &file, uint32_t query, bool expected) {
  std::cout << "Query: " << query << std::endl;
  bool failed = false;
  if (binary_search_basic(file, query) != expected) {
    std::cout << "binary_search_basic: Failed. Expected " << expected
              << std::endl;
    failed = true;
  }
  if (linear_search_basic(file, query) != expected) {
    std::cout << "linear_search_basic: Failed. Expected " << expected
              << std::endl;
    failed = true;
  }
  if (linear_search_vector(file, query) != expected) {
    std::cout << "linear_search_vector: Failed. Expected " << expected
              << std::endl;
    failed = true;
  }

  if (linear_search_basic_early_term(file, query) != expected) {
    std::cout << "linear_search_basic_early_term: Failed. Expected " << expected
              << std::endl;
    failed = true;
  }
  if (linear_search_vector_early_term(file, query) != expected) {
    std::cout << "linear_search_vector_early_term: Failed. Expected "
              << expected << std::endl;
    failed = true;
  }

  if (!failed) {
    std::cout << "Tests Passed." << std::endl;
  }
  std::cout << "==================================" << std::endl;
}

int main(int argc, char **argv) {

  if (argc < 3) {
    std::cerr << "you need to include a test name, query type, and query index"
              << std::endl;
    exit(1);
  }

  std::string case_filename = "tests/";
  case_filename += argv[1];
  case_filename += ".case";

  std::string quan_filename = "tests/";
  quan_filename += argv[1];
  quan_filename += ".quan";

  std::string pres_filename = "tests/";
  pres_filename += argv[1];
  pres_filename += ".pres";

  std::string non_filename = "tests/";
  non_filename += argv[1];
  non_filename += ".non";

  File test_file(case_filename.c_str());
  File quan_file(quan_filename.c_str());
  File pres_file(pres_filename.c_str());
  File non_file(non_filename.c_str());

  std::cout << std::boolalpha;

  // for printing the file
  // std::cout << "size: " << file.size(32) << std::endl;
  // for (size_t idx = 0; idx < file.size(32); ++idx) {
  //   std::cout << file.read32_reintc(idx) << std::endl;
  // }

  // size_t num_queries = 1;
  // std::vector<uint32_t> present_queries;
  // // present_queries.reserve(10);
  // std::vector<uint32_t> non_present_queries;
  // // non_present_queries.reserve(10);
  // for (size_t i = 0; i < num_queries; ++i) {
  //   present_queries.push_back(soln_file.read32_reintc(i));
  //   non_present_queries.push_back(soln_file.read32_reintc(10 + i));
  // }

  // std::cout << "present tests: " << std::endl;
  // for (auto q : present_queries) {
  //   test_all(test_file, q, true);
  // }
  // std::cout << "======================" << std::endl;
  // std::cout << "non-present tests: " << std::endl;
  // for (auto q : non_present_queries) {
  //   test_all(test_file, q, false);
  // }
  // std::cout << "======================" << std::endl;

  // just some hacky stuff to parameterise
  // size_t query_idx = (argv[2][0] == 'p') ? 0 : 20;
  uint32_t query_val;
  bool expected;
  size_t query_idx = (size_t)std::stoll(argv[3]);
  if (argv[2][0] == 'p') {
    query_val = pres_file.read32_reintc(query_idx);
    expected = true;
  } else if (argv[2][0] == 'q') {
    query_val = quan_file.read32_reintc(query_idx);
    expected = true;
  } else if (argv[2][0] == 'n') {
    query_val = non_file.read32_reintc(query_idx);
    expected = false;
  } else {
    std::cerr << "no query type specified" << std::endl;
    exit(1);
  }
  // binary_search_basic(test_file, query_val);
  // linear_search_basic(test_file, query_val);
  // linear_search_vector(test_file, query_val);
  // linear_search_vector_early_term(test_file, query_val);
  linear_search_basic_early_term(test_file, query_val);
  // test_all(test_file, query_val, expected);
}

// hyperfine --warmup 3 --parameter-scan iter 0 9 -D 1 --export-markdown
// linear_search_basic'./linear_search_basic n-chunked-s-256-w-uint32 p {iter}'

// clang++ - std = c++ 20 - O3 - march = native - o main search.cpp main.cpp
