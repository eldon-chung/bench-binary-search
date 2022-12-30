#!/bin/bash

file_name=$1

mkdir -p "results/$2$3"
output_file_name="results/$2$3/${file_name}-result.json"

# hyperfine --warmup 5 --export-json $output_file_name \
# 	-N \
# 	-L program binary_search_basic,linear_search_vector,linear_search_basic \
# 	"./{program} ${file_name} ${1} ${2}"

hyperfine --warmup 5 --export-json $output_file_name \
    -N \
    "./build/algos/linear_search_basic ${file_name} ${2} ${3}" \
    "./build/algos/linear_search_vector ${file_name} ${2} ${3}"\
    "./build/algos/linear_search_basic_early_term ${file_name} ${2} ${3}"\
    "./build/algos/linear_search_vector_early_term ${file_name} ${2} ${3}"\
    "./build/algos/binary_search_basic ${file_name} ${2} ${3}"
