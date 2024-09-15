#!/usr/bin/env bash

#------------------------------------#
# Copyright 2024 Nam Nguyen
# Licensed under Apache License v2.0
#------------------------------------#

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

failed=0; # number of failed cases

NBL_FILES=$(find tests -name '*.vila');

for nbl in $NBL_FILES; do
    # get expected output
    expected=${nbl}.expected;

    # run and get difference between output and expected output
    echo "Đang chạy test case $nbl...";
    if ! ./bin/vila $nbl | diff -u --color "$expected" -; then
        echo "Test case $nbl không đạt!";
        failed=$((failed + 1)); # count failed cases
    fi;
done;

if [ $failed -eq 0 ]; then
    echo;
    echo -e "${GREEN}Mọi test case đều đạt${NC}";
else
    echo;
    echo -e "${RED}Số test case không đạt: $failed${NC}";
fi;

exit $failed
