// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <cstdint>
#include <string>

namespace pyis {
enum SerializeType : char {
    BOOL = 1,
    UINT8,
    INT8,
    UINT16,
    INT16,
    UINT32,
    INT32,
    UINT64,
    INT64,
    FLOAT,
    DOUBLE,
    STRING,
    FILE,
    VECTOR,
    UNDEF
};

template <class T>
SerializeType get_type();

const char* get_type_name(SerializeType type);

template <class T>
const char* get_type_name();

extern const char* type_names[UNDEF];

template <>
inline SerializeType get_type<bool>() {
    return BOOL;
}

template <>
inline SerializeType get_type<uint8_t>() {
    return UINT8;
}

template <>
inline SerializeType get_type<int8_t>() {
    return INT8;
}

template <>
inline SerializeType get_type<uint16_t>() {
    return UINT16;
}

template <>
inline SerializeType get_type<int16_t>() {
    return INT16;
}

template <>
inline SerializeType get_type<uint32_t>() {
    return UINT32;
}

template <>
inline SerializeType get_type<int32_t>() {
    return INT32;
}

template <>
inline SerializeType get_type<uint64_t>() {
    return UINT64;
}

template <>
inline SerializeType get_type<int64_t>() {
    return INT64;
}

template <>
inline SerializeType get_type<float>() {
    return FLOAT;
}

template <>
inline SerializeType get_type<double>() {
    return DOUBLE;
}

template <>
inline SerializeType get_type<std::string>() {
    return STRING;
}

template <class T>
const char* get_type_name() {
    return get_type_name(get_type<T>());
}

}  // namespace pyis
