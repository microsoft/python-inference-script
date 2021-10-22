// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "binary_serialize_type.h"
#include "hardware_utils.h"

namespace pyis {

class BinarySerializeHelper {
  public:
    BinarySerializeHelper();
    template <class T>
    BinarySerializeHelper& add(const T& value);
    template <class T>
    BinarySerializeHelper& add(const std::vector<T>& vector);
    BinarySerializeHelper& add_file(const std::string& file_path);
    BinarySerializeHelper& add_file(const std::wstring& file_path);

    BinarySerializeHelper& clear();

    std::string serialize();

  private:
    template <class T>
    void serialize_value(const T& value);
    void serialize_file(std::ifstream& file);

    std::stringstream stream_;
};

template <class T>
BinarySerializeHelper& BinarySerializeHelper::add(const T& value) {
    serialize_value(get_type<T>());
    serialize_value(value);
    return *this;
}

template <class T>
BinarySerializeHelper& BinarySerializeHelper::add(const std::vector<T>& vector) {
    serialize_value(SerializeType::VECTOR);
    serialize_value(get_type<T>());
    serialize_value(vector.size());
    for (const auto& value : vector) {
        serialize_value(value);
    }
    return *this;
}

template <class T>
void BinarySerializeHelper::serialize_value(const T& value) {
    if (is_little_endian()) {
        stream_.write(reinterpret_cast<const char*>(&value), sizeof(T));
    } else {
        // due to most of computer is little endian, sorry big endian
        T new_value = value;
        swap_byte_order(reinterpret_cast<char*>(&new_value), sizeof(T));
        stream_.write(reinterpret_cast<const char*>(&new_value), sizeof(T));
    }
}

template <>
void BinarySerializeHelper::serialize_value(const std::string& value);

}  // namespace pyis