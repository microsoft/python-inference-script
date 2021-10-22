// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "binary_serialize_type.h"

namespace pyis {
const char* type_names[SerializeType::UNDEF] = {
    "unsigned char", "char",  "unsigned short", "short", "unsigned int", "int",  "unsigned long",
    "long",          "float", "double",         "str",   "vector",       "file", "undefined"};

const char* get_type_name(SerializeType type) {
    if (type > SerializeType::UNDEF) {
        type = SerializeType::UNDEF;
    }
    return type_names[type - 1];
}

}  // namespace pyis
