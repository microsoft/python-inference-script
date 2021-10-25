#!/bin/bash

tools/run-clang-format.sh -i
tools/run-cmake-format.py -i
tools/run-clang-tidy.sh