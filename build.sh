#!/bin/bash

# Clean old build directory and any root-level build files
echo "Removing old build artifacts..."
rm -f Makefile CMakeCache.txt cmake_install.cmake
rm -rf CMakeFiles/

# Remove old build directory if it exists
rm -rf build/

# Create fresh build directory
mkdir build/
cd build/

# Run CMake to configure the project
cmake ..

# Build the project
make

echo "Build completed!"