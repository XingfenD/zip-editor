#!/bin/bash

# 切换到run.sh所在目录
dirname "$0" | grep -q '^/' && cd "$(dirname "$0")" || cd "$(pwd)/$(dirname "$0")"

# 检查是否存在zip_demo.zip文件
if [ ! -f "zip_demos/zip_demo.zip" ]; then
    echo "zip_demo.zip不存在，开始创建..."

    # 创建zip_demo目录
    mkdir -p zip_demo

    # 在zip_demo目录中创建文件
echo "this is a test. from a1" > zip_demos/zip_demo/a1.txt
echo "this is b test. from a2.txt" > zip_demos/zip_demo/a2.txt
echo "ABCDEFGHIJKLMNOPQRSTUVWXYZ" > zip_demos/zip_demo/alphabet.txt

    # 将zip_demo目录压缩为zip文件
    zip -r zip_demos/zip_demo.zip zip_demos/zip_demo

    # 删除zip_demo目录
    rm -rf zip_demos/zip_demo

    echo "zip_demos/zip_demo.zip创建完成"
else
    echo "zip_demos/zip_demo.zip已存在，跳过创建步骤"
fi
