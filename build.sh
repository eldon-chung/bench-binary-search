#!/bin/bash

mkdir -p build/algos

# clang++ -std=c20++ -O3 -march=native -o main search.cpp main.cpp

for entry in src/algos/*.cpp ; do
    # echo $enty
    y=${entry%.cpp}
    file_name=${y##*/}

    echo compiling $file_name
    clang++ -std=c++20 -O3 -march=native -o build/algos/$file_name src/search.cpp $entry
done
