#!/bin/bash
rm -rf ./setup3
mkdir -p setup3

# 初始的 startId
startId=1

# 启动次数
count=5

# 启动程序的路径
program_path=$1 
#"./build/startGenerator"

# 循环启动程序
for (( i=1; i<=$count; i++ ))
do
    # 启动程序
    nohup $program_path 0 0 $startId 1000 1024 8000 100 > ./setup3/setup3_output_$i.txt 2>&1 &

    # 更新 startId
    startId=$((startId + 1000))
done

#nohup ./setup3.sh "./build/startGenerator" > setup3_output.log 2>&1 &
# ./setup3.sh "./build/startGenerator"