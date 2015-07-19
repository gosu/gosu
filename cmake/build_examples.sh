#!/bin/bash

cd ..
cd examples

cd Tutorial
mkdir -p build
cd build
cmake ..
make

cd ../..

cd TextInput
mkdir -p build
cd build
cmake ..
make

cd ../..

cd NetworkPong
mkdir -p build
cd build
cmake ..
make
