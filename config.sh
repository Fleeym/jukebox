#!/bin/bash

export SPLAT_DIR=$GEODE_SPLAT
export TOOLCHAIN=$GEODE_TOOLCHAIN
export HOST_ARCH=x86_64
export LLVM_VER=$GEODE_LLVM_VER
export CLANG_VER=$GEODE_CLANG_VER

if [ ! -d build ]; then
    mkdir build
fi
    
cd build && cmake \
    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_C_COMPILER=$GEODE_COMPILER \
    -DCMAKE_CXX_COMPILER=$GEODE_COMPILER \
    -G "Ninja" \
    ..
