#!/usr/bin/env python3

# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

import subprocess
import pathlib
import sys

patterns = [
    'CMakeLists.txt',
    # Keep an explicit list of files to format as we don't want to reformat
    # files we imported from other location.
    'pyis/**/CMakeLists.txt',
    'pyis/**/*.cmake',
    'third_party/**/CMakeLists.txt',
    'third_party/**/*.cmake',
    'py/**/CMakeLists.txt'
]

def find_cmake_files():
    here = pathlib.Path(__file__).parent.parent
    for pat in patterns:
        yield from here.glob(pat)

def run_cmake_format(paths, check_only=True):
    failed_cnt = 0
    for p in paths:
        # autosort is off because it breaks in cmake_format 5.1
        #   See: https://github.com/cheshirekow/cmake_format/issues/111
        print(f'check file: {p}')
        if check_only:
            cmd = ['cmake-format', '--check', '-l', 'info', '--autosort=false', p]
        else:
            cmd = ['cmake-format', '--in-place', '-l', 'info', '--autosort=false', p]

        try:
            subprocess.check_output(' '.join(cmd), stderr=subprocess.STDOUT, shell=True)
        except subprocess.CalledProcessError as e:
            failed_cnt += e.returncode
            print(str(e))
    return failed_cnt

if __name__ == "__main__":
    paths = [str(i) for i in list(find_cmake_files())]
    if "-i" in sys.argv:
        exit_code = run_cmake_format(paths, False)
    else:
        exit_code = run_cmake_format(paths, True)
    
    print(f"found issues in {exit_code} CMake files")
    sys.exit(exit_code)
