#!/bin/bash
rm -rf ./server_setup
mkdir -p server_setup

default_program_path="./build/startServer"

# 如果没有给定参数，则使用默认值
if [ $# -eq 0 ]; then
    program_path=$default_program_path
else
    program_path=$1
fi

# process port thread_nums
# 当Nonstop时，package_nums失效
nohup $program_path 8000 3 > ./server_setup/server_setup_output.txt 2>&1 &
