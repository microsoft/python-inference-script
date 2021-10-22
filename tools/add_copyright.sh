#!/bin/bash

for i in $(find pyis -name '*.h' -or -name '*.hpp' -or -name '*.c' -or -name '*.cc' -or -name '*.cxx' -or -name '*.cpp')
do
  if ! grep -q Copyright $i
  then
    echo $i
    cat tools/copyright_cpp.txt $i >$i.new && mv $i.new $i
  fi
done

for i in $(find py -name '*.py')
do
  if [[ $i == *__init__.py ]] ; then
    echo "skip __init__.py"
  elif ! grep -q Copyright $i ; then
    echo $i
    cat tools/copyright_py.txt $i >$i.new && mv $i.new $i   
  fi
done

for i in $(find docs/examples -name '*.py')
do
  if [[ $i == *__init__.py ]] ; then
    echo "skip __init__.py"
  elif ! grep -q Copyright $i ; then
    echo $i
    cat tools/copyright_py.txt $i >$i.new && mv $i.new $i   
  fi
done