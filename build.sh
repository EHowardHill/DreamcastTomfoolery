#!/bin/bash

set -e
simulant build dreamcast

# Cleanup before disc generation
mkdir -p ../cache
mv ./build/dreamcast-sh4-gcc/debug/monsters.elf ../monsters.elf
mv ./libraries/* ../cache/
mv ../cache/dreamcast-sh4-gcc ./libraries/dreamcast-sh4-gcc
rm -rf ./monsters/build/*

# Make disc
mkdcdisc -N -e ../monsters.elf -d . -o ../monsters.cdi