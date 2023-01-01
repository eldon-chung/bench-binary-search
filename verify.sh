#!/bin/bash



for entry in tests/*.case ; do

    y=${entry%.case}
    file_name=${y##*/}
    echo testing $file_name

    for i in {0..9} ; do
        if ! ./build/src/algos/verifier $file_name p $i ; then exit 1 ; fi
    done

    for i in {0..9} ; do
        if ! ./build/src/algos/verifier $file_name q $i ; then exit 1 ; fi
    done

    for i in {0..9} ; do 
        if ! ./build/src/algos/verifier $file_name n $i ; then exit 1 ; fi
    done
done
