#!/bin/bash
rm -rf ./setup1
mkdir -p setup1

startId=1
session_count=100
increment=400
times=1

# 10B, 128B, 1K, 16K, 32K, 64K,128K 
# 131072
# 启动程序的路径
program_path=$1 

for (( i=0; i<$times; i++ ))
do
   nohup $program_path 0 0 $startId $session_count 10240 8000 100 > ./setup1/setup1_output_$i.txt 2>&1 &
   startId=$((startId + increment))
done

#nohup ./setup1.sh "./build/startGenerator" > setup1_output_sum.log 2>&1 &

# 每10s记录一次，记录13小时
#nohup nmon -f -s 10 -c 4680 -F output.nmon > nmon_output.log &
