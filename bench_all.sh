#!/bin/bash

for i in {0..9} ; do
    ./bench.sh p $i
done

for i in {0..9} ; do
    ./bench.sh q $i
done

for i in {0..9} ; do 
    ./bench.sh n $i
done
