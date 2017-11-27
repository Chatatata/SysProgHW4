#!/bin/sh

make clean

make

mkdir -p ./build/rootdir

cp ../file.csv ./file.csv

./build/csvfs ./build/rootdir -d
