#pragma once

#include <stdint.h>

#include "File.h"

bool binary_search_basic(File const &file, uint32_t query);
bool linear_search_basic(File const &file, uint32_t query);
bool linear_search_vector(File const &file, uint32_t query);
bool linear_search_vector_early_term(File const &file, uint32_t query);
bool linear_search_basic_early_term(File const &file, uint32_t query);
bool linear_search_vector_early_term_twin_load(File const &file,
                                               uint32_t query);
bool linear_search_vector_twin_load(File const &file, uint32_t query);
