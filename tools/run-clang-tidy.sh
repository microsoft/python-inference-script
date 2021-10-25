#!/bin/bash

BUILD_FOLDER=build_linux/

if test -d "build_linux"; then
    BUILD_FOLDER=build_linux/
elif test -d "build"; then
    BUILD_FOLDER=build/
fi

python3 tools/run-clang-tidy.py -p ${BUILD_FOLDER} \
    'pyis\/.*' \
    'tests\/.*' \
    '!pyis\/ops\/linear_chain_crf\/.*'
