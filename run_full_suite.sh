#!/bin/bash

echo "=== Compiling (-O0) ==="
./auxiliary-compilation-scripts/compile_O0.sh
echo

echo "=== Compiling (-O3) ==="
./auxiliary-compilation-scripts/compile_O3.sh
echo

echo "=== Running: scalar (-O0) ==="
./main_O0 --nosimd
echo

echo "=== Running: SIMD (-O0) ==="
./main_O0 --simd
echo

echo "=== Running: scalar (-O3) ==="
./main_O3 --nosimd
echo

echo "=== Running: SIMD (-O3) ==="
./main_O3 --simd
echo
