# # bash program to compile and run the c file by passing arguments from 1 to 100 and recording avg run time for each call

#!/bin/bash

total_time=0
gcc factorial.c -o a.out
for i in {1..100}; do
    runtime=$( /usr/bin/time -f "%e" ./a.out $i 2>&1 )
    total_time=$(bc <<< "$total_time + $runtime")
    echo "Run $i: $runtime seconds"
done

average_time=$(bc <<< "scale=3; $total_time / 100")
echo "Average time for 100 runs: $average_time seconds"
