// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "hardware_utils.h"

#include <algorithm>
#include <cstdint>

union EndianDetector {
    uint32_t i_;
    char str_[sizeof(uint32_t)];
};

static const EndianDetector DETECTOR = {0x01020304};

bool is_little_endian() { return DETECTOR.str_[0] == 4; }

void swap_byte_order(char* data, int size) {
    int i = 0;
    int j = size - 1;
    while (i < j) {
        std::swap(data[i++], data[j--]);
    }
}
