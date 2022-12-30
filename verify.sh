#!/bin/bash

if [ $# -eq 0 ]; then
    echo "You need to provide a test case"
    exit 1
fi


for i in {0..9} ; do
    ./build/algos/verifier ${1} p $i
done

for i in {0..9} ; do
    ./build/algos/verifier ${1} q $i
done

for i in {0..19} ; do 
    ./build/algos/verifier ${1} n $i
done
