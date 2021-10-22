// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "word_dict.h"

#include "pyis/share/json_persist_helper.h"
#include "pyis/share/model_storage_local.h"

namespace pyis {
namespace ops {

WordDict::WordDict() : thread_num_(1) {}

WordDict::WordDict(const std::string& data_file) : WordDict() {
    ModelStorageLocal fs;
    Load(data_file, fs);
}

void WordDict::Load(const std::string& data_file, ModelStorage& storage) {
    auto istream = storage.open_istream(data_file);
    std::string line;
    while (std::getline(*istream, line)) {
        line = trim_str(line);
        if (line.length() == 0) {
            continue;
        }
        std::vector<std::string> tokens = split_str(line);
        mapping_[tokens[0]] = tokens[1];
    }
}

void WordDict::Save(const std::string& data_file, ModelStorage& storage) {
    auto ostream = storage.open_ostream(data_file);
    for (auto& kv : mapping_) {
        ostream->write(kv.first.c_str(), kv.first.length());
        ostream->write(" ", 1);
        ostream->write(kv.second.c_str(), kv.second.length());
        ostream->write("\n", 1);
    }
}

std::vector<std::string> WordDict::Translate(const std::vector<std::string>& tokens) {
    std::vector<std::string> res;
    res.reserve(tokens.size());

    for (const auto& i : tokens) {
        if (mapping_.count(i) == 0) {
            res.push_back(i);

        } else {
            res.push_back(mapping_[i]);
        }
    }

    return res;
}

std::string WordDict::Serialize(ModelStorage& storage) {
    // Step 1. (optional) write massive data into external files if  there is
    // Notice a few things:
    // - For files, do read/write them via `ModelStorage` APIs instead of native
    //   file system APIs, because the model might not be serialized as usual
    //   local files on-disk. For example, we may use archived/compressed files
    //   or even remote network files.
    // - Use `uniq_file` to get a non-existing file. The API will return
    //   `{variant}[,1,2,...]{suffix}` until a file doesn't exist.
    //   For example, it will check `word_dict.data.txt`, `word_dict1.data.txt`,
    //   `word_dict2.data.txt`, ..., for existence in order.
    std::string data_file = storage.uniq_file("word_dict", ".data.txt");
    Save(data_file, storage);

    // Step 2. (required) encode all states into a json object
    JsonPersistHelper jph(1);
    jph.add_file("data", data_file);
    jph.add("thread_num", 1, true);  // thread num is configurable at runtime.

    // Step 3. serialize the states
    // Notice a few things:
    // - If there isn't any configurable, use the following API to save states:
    //   std::string state = jph.serialize();
    // - If there are configurables, e.g., `thread_num`, we will need to generate
    //   a json configuration file. We will have the chance to set them with
    //   appropriate values before loading the model.
    std::string config_file = storage.uniq_file("word_dict", ".config.json");
    std::string state = jph.serialize(config_file, storage);
    return state;
}

void WordDict::Deserialize(const std::string& state, ModelStorage& storage) {
    JsonPersistHelper jph(state, storage);
    int version = jph.version();

    // check version for backward compatibility
    if (1 == version) {
        std::string data_file = jph.get_file("data");
        Load(data_file, storage);
        return;
    }

    PYIS_THROW("WordDict v%d is incompatible with the runtime", version);
}

}  // namespace ops
}  // namespace pyis