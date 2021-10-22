// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "binary_serialize_helper.h"

#include <codecvt>
#include <locale>

#include "exception.h"
#include "str_utils.h"

namespace pyis {

const uint8_t SERIALIZE_PROTO_VERSION = 1;

BinarySerializeHelper::BinarySerializeHelper() : stream_(std::ios::in | std::ios::out) {
    serialize_value(SERIALIZE_PROTO_VERSION);
}

void BinarySerializeHelper::serialize_file(std::ifstream& file) {
    // get file length
    file.seekg(0, std::ios_base::end);
    size_t length = file.tellg();
    serialize_value(length);

    file.seekg(0, std::ios_base::beg);
    stream_ << file.rdbuf();
}

BinarySerializeHelper& BinarySerializeHelper::add_file(const std::string& file_path) {
#if _WIN32
    std::ifstream file(str_to_wstr(file_path), std::ios::binary | std::ios::in);
#else
    std::ifstream file(file_path, std::ios::binary | std::ios::out | std::ios::in);
#endif
    if (!file) {
        PYIS_THROW("failed to open file %s", file_path.c_str());
    }

    serialize_value(SerializeType::FILE);
    serialize_file(file);
    file.close();

    return *this;
}

BinarySerializeHelper& BinarySerializeHelper::add_file(const std::wstring& file_path) {
#if _WIN32
    std::ifstream file(file_path, std::ios::binary | std::ios::in | std::ios::out);
#else
    std::ifstream file(wstr_to_str(file_path), std::ios::binary | std::ios::out | std::ios::in);
#endif

    if (!file) {
        PYIS_THROW("failed to open file %s", wstr_to_str(file_path).c_str());
    }

    serialize_value(SerializeType::FILE);
    serialize_file(file);
    file.close();

    return *this;
}

BinarySerializeHelper& BinarySerializeHelper::clear() {
    stream_.clear();
    return *this;
}

std::string BinarySerializeHelper::serialize() { return stream_.str(); }

template <>
void BinarySerializeHelper::serialize_value(const std::string& value) {
    serialize_value(value.length());
    stream_.write(value.c_str(), static_cast<std::streamsize>(value.length()));
}

}  // namespace pyis