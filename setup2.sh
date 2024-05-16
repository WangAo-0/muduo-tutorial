#!/bin/bash
rm -rf ./setup2
mkdir -p setup2

# 初始的 startId
startId=4001

# 启动次数
count=12

# 启动程序的路径
program_path=$1 
#"./build/startGenerator"

# 循环启动程序
for (( i=1; i<=$count; i++ ))
do
    # 启动程序
    nohup $program_path 0 1 $startId 200 1000 8000 100 > ./setup2/setup2_output_$i.txt 2>&1 &

    # 更新 startId
    startId=$((startId + 200))

    # 如果还需要启动，那么等待1小时
    if (( i < $count ))
    then
        sleep 3600
    fi
done

#nohup ./setup2.sh "./build/startGenerator" > setup2_output.log 2>&1 &