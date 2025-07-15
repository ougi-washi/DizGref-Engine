#! /bin/bash
mkdir -p build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build -S .
make -C build
