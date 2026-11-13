#!/bin/bash

if [[ $# -eq 0 ]] ; then
    echo "./test_sequential.sh <testfile>"
    exit 0
fi

if [[ "$1" != *.test ]] ; then
    echo "test file should have .test extension."
    exit 0
fi

test_file=$1
bin_file="${test_file%.test}.bin"

test_data=$(grep -v '\.bin' "$test_file")
exec_data=$(./rv64-emu "$bin_file" 2>&1)

if [[ "$2" == "-p" ]] ; then
    test_data=$(printf '%s\n' "$test_data" | head -n -2)
    exec_data=$(printf '%s\n' "$exec_data" | head -n -3)
fi

diff --color -u <(printf '%s\n' "$exec_data") <(printf '%s\n' "$test_data")

if [ $? == 0 ] ; then
    echo "$test_file sequential passed."
else
    echo "$test_file sequential failed."
fi


test_data=$(grep -v '\.bin' "$test_file")
exec_data=$(./rv64-emu -p "$bin_file" 2>&1)
test_data=$(printf '%s\n' "$test_data" | head -n -2)
exec_data=$(printf '%s\n' "$exec_data" | head -n -3)

diff --color -u <(printf '%s\n' "$exec_data") <(printf '%s\n' "$test_data")

if [ $? == 0 ] ; then
    echo "$test_file pipelined passed."
else
    echo "$test_file pipelined failed."
fi