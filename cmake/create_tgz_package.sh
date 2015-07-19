#!/bin/bash

mkdir build
cd build
cmake ..
cpack -G TGZ
