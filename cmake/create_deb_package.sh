mkdir -p build
cd build
cmake ..
make
cpack -G DEB
