#!/usr/bin/env sh

if ! make -j15; then
    exit 1
fi
cd fth
./pforth -i system.fth
cd ..
