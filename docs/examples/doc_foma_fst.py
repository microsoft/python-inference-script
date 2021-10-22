# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

from pyis.python import ops
from typing import List

# build foma fst
# build foma fst is via its script language, the language spec can be found:
fst_infile_src = \
r"""
define Number ["0"|1|2|3|4|5|6|7|8|9] ;
define Day [(1|2) Number | 3 "0" | 3 1];
define Year Number (Number) (Number) (Number); # Could use Number^<5

define WeekDay ["Monday"|"Tuesday"|"Wednesday"|"Thursday"|"Friday"|"Saturday"|"Sunday"];
define Month ["January"|"February"|"March"|"April"|"May"|"June"|"July"|"August"|"September"|"October"|"November"|"December"];
define RegDates [WeekDay | Month " " Day (", " Year)];

define DateParser [RegDates @-> "<DATE>" ... "</DATE>"];

regex DateParser;
apply down April 14, 2010 – Nelson Mandela was honoured
save stack temp.fst
"""

# you can also save the script into a file, and call `compile_from_file` instead
ops.FomaFst.compile_from_str(fst_infile_src) 

# instantiate a FomaFst object by loading a compiled fst
fst = ops.FomaFst("temp.fst") 

# use foma fst
# apply_down(input_str) will apply the input_str to the fst on top of the stack
# note: `regex DateParser` push the DateParser to the top of the stack
assert(fst.apply_down("April 14, 2010 – Nelson Mandela was honoured") == \
    "<DATE>April 14, 2010</DATE> – Nelson Mandela was honoured")
