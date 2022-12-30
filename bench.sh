#!/bin/bash

mkdir -p "results/$1$2"

for entry in tests/*.case
do
    y=${entry%.case}
    file_name=${y##*/}

	output_file_name="results/$1$2/${file_name}-result.json"

	# hyperfine --warmup 5 --export-json $output_file_name \
	# 	-N \
	# 	-L program binary_search_basic,linear_search_vector,linear_search_basic \
	# 	"./{program} ${file_name} ${1} ${2}"
	
	hyperfine --warmup 5 --export-json $output_file_name \
		-N \
		"./linear_search_basic ${file_name} ${1} ${2}" \
		"./linear_search_vector ${file_name} ${1} ${2}"\
		"./linear_search_basic_early_term ${file_name} ${1} ${2}"\
		"./linear_search_vector_early_term ${file_name} ${1} ${2}"\
		"./binary_search_basic ${file_name} ${1} ${2}"
done


