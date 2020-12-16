#!/usr/bin/env bash

set -eu

# clangd works better with clang:

export CC=clang
export CXX=clang++

mkdir -p build
(
    cd build &&
        cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
              -DCMAKE_C_COMPILER=$CC \
              -DCMAKE_CXX_COMPILER=$CXX &&
        cmake --build . -- -j
)
if [[ ! -e compile_commands.json ]]; then
    ln -s $(realpath build/compile_commands.json) compile_commands.json
fi
