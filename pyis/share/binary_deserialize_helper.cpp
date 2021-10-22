// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "binary_deserialize_helper.h"

#include <codecvt>
#include <fstream>
#include <locale>

#include "exception.h"
#include "str_utils.h"

static const size_t FILE_BUF_SIZE = 1 << 10;

namespace pyis {

BinaryDeserializeHelper::BinaryDeserializeHelper(const std::string& str) : stream_(str, std::ios_base::in) {
    deserialize_value(proto_version_);
}

BinaryDeserializeHelper::BinaryDeserializeHelper(std::string&& str) : stream_(str, std::ios_base::in) {
    deserialize_value(proto_version_);
}

BinaryDeserializeHelper& BinaryDeserializeHelper::get_file(const std::string& file_path) {
    check_type(SerializeType::FILE);

#if _WIN32
    std::ofstream file(str_to_wstr(file_path), std::ios::binary | std::ios::out);
#else
    std::ofstream file(file_path, std::ios::binary | std::ios::out);
#endif
    if (!file) {
        PYIS_THROW("failed to open file %s", file_path.c_str());
    }

    deserialize_file(file);
    file.close();

    return *this;
}

BinaryDeserializeHelper& BinaryDeserializeHelper::get_file(const std::wstring& file_path) {
    check_type(SerializeType::FILE);

#if _WIN32
    std::ofstream file(file_path, std::ios::binary | std::ios::out);
#else
    std::ofstream file(wstr_to_str(file_path), std::ios::binary | std::ios::out);
#endif
    if (!file) {
        PYIS_THROW("failed to open file %s", wstr_to_str(file_path).c_str());
    }

    deserialize_file(file);
    file.close();

    return *this;
}

BinaryDeserializeHelper& BinaryDeserializeHelper::get_file_content(std::string& content) {
    check_type(SerializeType::FILE);
    // the format of file is same with file
    deserialize_value(content);

    return *this;
}

void BinaryDeserializeHelper::deserialize_file(std::ofstream& file) {
    size_t length;
    deserialize_value(length);

    char buf[FILE_BUF_SIZE];
    while (length != 0) {
        auto copy_size = static_cast<std::streamsize>(std::min(length, FILE_BUF_SIZE));
        stream_.read(buf, copy_size);
        file.write(buf, copy_size);
        length -= copy_size;
    }
}

void BinaryDeserializeHelper::check_type(SerializeType expected_type) {
    SerializeType type;

    deserialize_value(type);
    if (type != expected_type) {
        PYIS_THROW("type mismatch, expect %s, got %s", get_type_name(type), get_type_name(expected_type));
    }
}

void BinaryDeserializeHelper::check_vector_elem_type(SerializeType expected_type) {
    SerializeType elem_type;

    deserialize_value(elem_type);
    if (elem_type != expected_type) {
        PYIS_THROW("vector elemement mismatch, expect %s, got %s", get_type_name(elem_type),
                   get_type_name(expected_type));
    }
}

template <>
void BinaryDeserializeHelper::deserialize_value(std::string& value) {
    size_t size = 0;
    deserialize_value(size);

    value.resize(size);
    stream_.read(&value[0], static_cast<std::streamsize>(size));
}
}  // namespace pyis