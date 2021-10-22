// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "json_persist_helper.h"

#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>

#include <iomanip>
#include <iostream>
#include <map>

#include "exception.h"
#include "str_utils.h"

namespace pyis {

JsonPersistHelper::JsonPersistHelper() : doc_(rapidjson::kObjectType), allocator_(doc_.GetAllocator()) {}

JsonPersistHelper::JsonPersistHelper(int version) : JsonPersistHelper() { add("version", version, false); }

JsonPersistHelper::JsonPersistHelper(const std::string& state) : JsonPersistHelper() { deserialize(state); }

JsonPersistHelper::JsonPersistHelper(const std::string& state, ModelStorage& storage) : JsonPersistHelper() {
    deserialize(state, storage);
}

JsonPersistHelper& JsonPersistHelper::add(const std::string& key, const std::string& value, bool configurable) {
    rapidjson::Value k(key.c_str(), allocator_);
    rapidjson::Value v(value.c_str(), allocator_);
    doc_.AddMember(k, v, allocator_);

    if (configurable) {
        configurables_.insert(key);
    }

    return *this;
}

JsonPersistHelper& JsonPersistHelper::add(const std::string& key, const std::vector<std::string>& value,
                                          bool configurable) {
    rapidjson::Value arr(rapidjson::kArrayType);

    for (const auto& i : value) {
        rapidjson::Value item(i.c_str(), allocator_);
        arr.PushBack(item, allocator_);
    }

    rapidjson::Value k(key.c_str(), allocator_);
    doc_.AddMember(k, arr, allocator_);

    if (configurable) {
        configurables_.insert(key);
    }

    return *this;
}

JsonPersistHelper& JsonPersistHelper::add_file(const std::string& key, const std::string& file, bool configurable) {
    std::string key_type = key + ":file";
    JsonPersistHelper::add(key_type, file);

    if (configurable) {
        configurables_.insert(key);
    }

    return *this;
}

std::string JsonPersistHelper::get(const std::string& key) {
    if (!doc_.HasMember(key.c_str())) {
        PYIS_THROW("key not found in json object. key: %s", key.c_str());
    }
    const rapidjson::Value& value = doc_[key.c_str()];
    return std::string(value.GetString());
}

std::string JsonPersistHelper::get_file(const std::string& key) {
    std::string key_file = key + ":file";
    return get(key_file);
}

std::string JsonPersistHelper::serialize() {
    rapidjson::Document non_configurable_doc(rapidjson::kObjectType);

    for (auto& m : doc_.GetObject()) {
        rapidjson::Value k(m.name, non_configurable_doc.GetAllocator());
        rapidjson::Value v(m.value, non_configurable_doc.GetAllocator());
        if (configurables_.count(m.name.GetString()) == 0) {
            non_configurable_doc.AddMember(k, v, non_configurable_doc.GetAllocator());
        }
    }

    rapidjson::StringBuffer buffer;
    buffer.Clear();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    non_configurable_doc.Accept(writer);
    return std::string(buffer.GetString());
}

std::string JsonPersistHelper::serialize(const std::string& config_file, ModelStorage& storage) {
    rapidjson::Document configurable_doc(rapidjson::kObjectType);

    for (auto& m : doc_.GetObject()) {
        rapidjson::Value k(m.name, configurable_doc.GetAllocator());
        rapidjson::Value v(m.value, configurable_doc.GetAllocator());
        if (configurables_.count(m.name.GetString()) != 0) {
            configurable_doc.AddMember(k, v, configurable_doc.GetAllocator());
        }
    }

    if (configurable_doc.MemberCount() > 0) {
        char buffer[10240];
        auto fp = storage.open_file(config_file.c_str(), "wb");
        rapidjson::FileWriteStream os(fp.get(), buffer, sizeof(buffer));
        rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
        configurable_doc.Accept(writer);

        add("config:file", config_file, false);
    }

    std::string res = serialize();

    // remove config:file, it is not needed, and it will interfere signing as well
    if (doc_.HasMember("config:file")) {
        doc_.RemoveMember("config:file");
    }

    return res;
}

void JsonPersistHelper::deserialize(const std::string& state) {
    if (doc_.Parse(state.c_str()).HasParseError()) {
        PYIS_THROW("failed to parse from json string. offset:%zu, message:%s", doc_.GetErrorOffset(),
                   GetParseError_En(doc_.GetParseError()));
    }
}

void JsonPersistHelper::deserialize(const std::string& state, ModelStorage& storage) {
    deserialize(state);

    if (doc_.HasMember("config:file")) {
        rapidjson::Document configurable_doc(rapidjson::kObjectType);
        auto fp = storage.open_file(doc_["config:file"].GetString(), "rb");
        char read_buffer[10240];
        rapidjson::FileReadStream is(fp.get(), read_buffer, sizeof(read_buffer));
        configurable_doc.ParseStream(is);

        for (auto& m : configurable_doc.GetObject()) {
            if (doc_.HasMember(m.name.GetString())) {
                doc_.RemoveMember(m.name.GetString());
            }
            rapidjson::Value k(m.name, allocator_);
            rapidjson::Value v(m.value, allocator_);
            configurables_.insert(m.name.GetString());
            doc_.AddMember(k, v, allocator_);
        }

        // remove config:file, it is not needed, and it will interfere signing as well
        doc_.RemoveMember("config:file");
    }
}

int JsonPersistHelper::version() { return get<int>("version"); }

enum JsonNodeType {
    NONE = 0,
    BOOL,
    INT,
    UINT,
    INT64,
    UINT64,
    FLOAT,
    DOUBLE,
    STR,
    ARR,
    DICT,
    FILE,
};

void JsonPersistHelper::sign(rapidjson::Value* obj, boost::uuids::detail::md5& md5, ModelStorage* storage) {
    if (obj->IsBool()) {
        JsonNodeType t = JsonNodeType::BOOL;
        md5.process_bytes(&t, sizeof(t));
        int v = static_cast<int>(obj->GetBool());
        md5.process_bytes(&v, sizeof(v));
    } else if (obj->IsInt()) {
        JsonNodeType t = JsonNodeType::INT;
        md5.process_bytes(&t, sizeof(t));
        int v = obj->GetInt();
        md5.process_bytes(&v, sizeof(v));
    } else if (obj->IsUint()) {
        JsonNodeType t = JsonNodeType::UINT;
        md5.process_bytes(&t, sizeof(t));
        unsigned v = obj->GetUint();
        md5.process_bytes(&v, sizeof(v));
    } else if (obj->IsInt64()) {
        JsonNodeType t = JsonNodeType::INT64;
        md5.process_bytes(&t, sizeof(t));
        int64_t v = obj->GetInt64();
        md5.process_bytes(&v, sizeof(v));
    } else if (obj->IsUint64()) {
        JsonNodeType t = JsonNodeType::UINT64;
        md5.process_bytes(&t, sizeof(t));
        uint64_t v = obj->GetUint64();
        md5.process_bytes(&v, sizeof(v));
    } else if (obj->IsFloat()) {
        JsonNodeType t = JsonNodeType::FLOAT;
        md5.process_bytes(&t, sizeof(t));
        float v = obj->GetFloat();
        md5.process_bytes(&v, sizeof(v));
    } else if (obj->IsDouble()) {
        JsonNodeType t = JsonNodeType::DOUBLE;
        md5.process_bytes(&t, sizeof(t));
        double v = obj->GetDouble();
        md5.process_bytes(&v, sizeof(v));
    } else if (obj->IsString()) {
        JsonNodeType t = JsonNodeType::STR;
        md5.process_bytes(&t, sizeof(t));
        std::string v(obj->GetString());
        size_t len = v.length();
        md5.process_bytes(&len, sizeof(len));
        md5.process_bytes(v.c_str(), v.length());
    } else if (obj->IsArray()) {
        JsonNodeType t = JsonNodeType::ARR;
        md5.process_bytes(&t, sizeof(t));
        size_t size = obj->GetArray().Size();
        md5.process_bytes(&size, sizeof(size));

        for (auto& v : obj->GetArray()) {
            sign(&v, md5, storage);
        }
    } else if (obj->IsObject()) {
        JsonNodeType t = JsonNodeType::DICT;
        md5.process_bytes(&t, sizeof(t));
        size_t size = obj->GetObject().MemberCount();
        md5.process_bytes(&size, sizeof(size));

        // sort keys
        std::map<std::string, rapidjson::Value*> sorted_kvs;
        for (auto& m : obj->GetObject()) {
            std::string key(m.name.GetString());
            sorted_kvs[key] = &m.value;
        }
        for (auto& m : sorted_kvs) {
            // key
            JsonNodeType t = JsonNodeType::STR;
            if (str_ends_with(m.first, ":file")) {
                t = JsonNodeType::FILE;
                if (storage == nullptr) {
                    PYIS_THROW("a file needs to be signed, but a ModelStorage instance is not provided. file:%s",
                               m.second->GetString());
                }
            }
            md5.process_bytes(&t, sizeof(t));
            size_t len = m.first.length();
            md5.process_bytes(&len, sizeof(len));
            md5.process_bytes(m.first.c_str(), m.first.length());

            // value
            if (str_ends_with(m.first, ":file")) {
                sign_file(m.second->GetString(), *storage, md5);
            } else {
                sign(m.second, md5, storage);
            }
        }
    } else if (obj->IsNull()) {
        JsonNodeType t = JsonNodeType::NONE;
        md5.process_bytes(&t, sizeof(t));
    } else {
        PYIS_THROW("json node type %d is unsupported for signing", obj->GetType());
    }
}

void JsonPersistHelper::sign_file(const std::string& file_path, ModelStorage& storage, boost::uuids::detail::md5& md5) {
    auto f = storage.open_file(file_path.c_str(), "rb");
    unsigned char buffer[1024];
    int num_read;
    while ((num_read = std::fread(buffer, 1, sizeof(buffer), f.get())) == sizeof(buffer)) {
        md5.process_bytes(buffer, num_read);
    }
    md5.process_bytes(buffer, num_read);  // the remaining
}

std::string JsonPersistHelper::sign(ModelStorage* storage) {
    boost::uuids::detail::md5 md5;
    sign(&doc_, md5, storage);

    boost::uuids::detail::md5::digest_type digest;
    md5.get_digest(digest);

    std::stringstream ss;
    for (unsigned int i : digest) {
        ss << std::setfill('0') << std::setw(8) << std::hex << i;
    };
    return ss.str();
}

}  // namespace pyis