#!/bin/bash

mkdir -p build
cd build
cmake ..
cpack -G TGZ
