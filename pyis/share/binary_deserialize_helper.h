// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <sstream>
#include <string>
#include <vector>

#include "binary_serialize_type.h"
#include "hardware_utils.h"

namespace pyis {

class BinaryDeserializeHelper {
  public:
    explicit BinaryDeserializeHelper(const std::string& str);
    explicit BinaryDeserializeHelper(std::string&& str);
    template <class T>
    BinaryDeserializeHelper& get(T& value);
    template <class T>
    BinaryDeserializeHelper& get(std::vector<T>& vector);

    // write the file to the file path
    BinaryDeserializeHelper& get_file(const std::string& file_path);
    BinaryDeserializeHelper& get_file(const std::wstring& file_path);
    BinaryDeserializeHelper& get_file_content(std::string& content);

  private:
    template <class T>
    void deserialize_value(T& value);
    void deserialize_file(std::ofstream& file);

    void check_type(SerializeType expected_type);
    void check_vector_elem_type(SerializeType expected_type);

    std::stringstream stream_;
    uint8_t proto_version_;
};

template <class T>
BinaryDeserializeHelper& BinaryDeserializeHelper::get(T& value) {
    check_type(get_type<T>());

    deserialize_value<T>(value);
    return *this;
}

template <class T>
BinaryDeserializeHelper& BinaryDeserializeHelper::get(std::vector<T>& vector) {
    check_type(SerializeType::VECTOR);
    check_vector_elem_type(get_type<T>());

    size_t size = 0;
    deserialize_value(size);
    vector.resize(size);
    for (int i = 0; i < size; i++) {
        deserialize_value(vector[i]);
    }

    return *this;
}

template <class T>
void BinaryDeserializeHelper::deserialize_value(T& value) {
    stream_.read(reinterpret_cast<char*>(&value), sizeof(T));

    // the serialized order is little endian, swap the byte order
    if (!is_little_endian()) {
        swap_byte_order(reinterpret_cast<char*>(&value), sizeof(T));
    }
}

template <>
void BinaryDeserializeHelper::deserialize_value(std::string& value);

}  // namespace pyis