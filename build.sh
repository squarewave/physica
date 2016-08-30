#!/bin/bash

COMMON_COMPILER_FLAGS="-rdynamic -O2 -g `sdl2-config --cflags --libs` -lGLEW -lGL -lm -std=c++11 -Werror -Weverything -Wno-writable-strings -Wno-old-style-cast -Wno-missing-field-initializers -Wno-missing-braces -Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-missing-prototypes -Wno-c++98-compat-pedantic -Wno-c99-extensions -Wno-padded -Wno-missing-noreturn -Wno-reserved-id-macro -Wno-unused-parameter -Wno-global-constructors -Wno-cast-align"
# COMMON_LINKER_FLAGS="-lsdl2 -lopengl -lglew"

node build_animations.js

mkdir -p build
pushd build

clang++ ../main.cpp -o physica $COMMON_COMPILER_FLAGS
popd
