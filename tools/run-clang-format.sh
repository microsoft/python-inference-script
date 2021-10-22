#!/bin/bash

PYTHON=python3

while getopts ":hi" arg; do
  case $arg in
    i)
      ${PYTHON} tools/run-clang-format.py --in-place --recursive "pyis" "tests"
      exit $?
      ;;
    h | *) # display help.
      echo "Usage: $0 [-i]" >&2
      exit 1
      ;;
  esac
done

# perform noraml clang foramt check
${PYTHON} tools/run-clang-format.py --recursive "pyis" "tests"
exit $?