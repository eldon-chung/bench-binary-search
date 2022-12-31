#!/bin/bash

for i in {0..9} ; do
    ./bench p $i
done

for i in {0..9} ; do
    ./bench q $i
done

for i in {0..19} ; do 
    ./bench n $i
done
