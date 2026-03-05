#!/bin/bash
set -e

CXX=g++
CXXFLAGS="-O3 -march=native -Wall -Wextra"
INCLUDES="-Iinclude -Iinclude/CAR-practica2"
LIBS="-lssl -lcrypto"

SRC="src/convolution.cpp src/image.cpp test/test_hash_images.cpp"
OUT="hash_test"

echo "Compiling..."
$CXX $CXXFLAGS $INCLUDES $SRC -o $OUT $LIBS

echo "Running..."
./$OUT
