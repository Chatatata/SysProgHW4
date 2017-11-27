#!/bin/sh

make clean

make

mkdir -p ./build/rootdir

cp ./file.csv ./build/file.csv

./build/csvfs ./build/rootdir -d
