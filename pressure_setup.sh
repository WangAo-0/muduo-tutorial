#!/bin/bash
rm -rf ./pressure_setup
mkdir -p pressure_setup

default_program_path="./build/startGenerator"

# 如果没有给定参数，则使用默认值
if [ $# -eq 0 ]; then
    program_path=$default_program_path
else
    program_path=$1
fi

startId=1
# 10B, 128B, 1K, 16K, 32K, 64K,128K 
# 131072

# process thread_nums stop startId client_nums package_size port package_nums
# 当Nonstop时，package_nums失效
nohup $program_path 3 0 $startId 1000 1024 8000 100 > ./pressure_setup/pressure_setup_output.txt 2>&1 &