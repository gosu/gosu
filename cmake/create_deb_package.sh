#!/bin/bash

mkdir -p build
cd build
rm *.deb
cmake ..
make
cpack -G DEB
