#!/bin/bash

set -e

if [ ! -d `pwd`/build]; then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build && 
    cmake .. &&
    make -j4

cd ..

# 把头文件拷贝到 /usr/include/mymuduo so库拷贝到 /usr/lib PATH
if [ ! -d "/usr/include/mymuduo/net" ];then
    mkdir /usr/include/mymuduo/net
fi

if [ ! -d "/usr/include/mymuduo/tools" ];then
    mkdir /usr/include/mymuduo/tools
fi

for header in `ls ./src/net/*.h`
do
    cp $header /usr/include/mymuduo/net/
done

for header in `ls ./src/tools/*.h`
do
    cp $header /usr/include/mymuduo/tools/
done

cp `pwd`/lib/libmymuduo.so /usr/lib

#刷新动态库缓存
ldconfig