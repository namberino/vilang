#!/usr/bin/env bash

#------------------------------------#
# Copyright 2024 Nam Nguyen
# Licensed under Apache License v2.0
#------------------------------------#

declare -A elapsed_times # associative array (basically a dictionary)

NBL_FILES=$(find benchmark -name '*.vila')

for program in $NBL_FILES; do
    echo "Đang chạy benchmark: $program..."

    # run and capture output
    output=$(./bin/vila "$program")
    
    # extract elapsed time
    elapsed_time=$(echo "$output" | grep 'Elapsed:' | awk '{print $2}')
    
    # get program name without the path
    program_name=$(basename "$program")
    
    # store data in dictionary
    elapsed_times["$program_name"]="$elapsed_time"
done

echo
echo "Thời gian chạy của các benchmark:"

for program in "${!elapsed_times[@]}"; do
    echo "$program: ${elapsed_times[$program]}"
done
