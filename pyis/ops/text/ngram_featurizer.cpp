// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "ngram_featurizer.h"

#include <fstream>  // std::ifstream
#include <sstream>

#include "pyis/share/exception.h"
#include "pyis/share/json_persist_helper.h"
#include "pyis/share/scope_guard.h"
#include "pyis/share/str_utils.h"

namespace pyis {
namespace ops {

const std::string NGramFeaturizer::BOS_MARK = "BeginningOfDoc";
const std::string NGramFeaturizer::EOS_MARK = "EndOfDoc";

NGramFeaturizer::NGramFeaturizer(int order, bool boundaries) : order_(order), boundaries_(boundaries), next_id_(0) {
    if (order_ > 8 || order_ <= 0) {
        PYIS_THROW("supported ngram length is [1, 8]");
    }
}

void NGramFeaturizer::Fit(const std::vector<std::string>& tokens) {
    std::vector<std::string> new_tokens(tokens);
    new_tokens.insert(new_tokens.begin(), NGramFeaturizer::BOS_MARK);
    new_tokens.push_back(NGramFeaturizer::EOS_MARK);

    auto token_count = static_cast<int>(new_tokens.size());

    // A query should contain at least bos + eos + at least #order tokens
    if (token_count < order_) {
        return;
    }

    // bos
    if (boundaries_) {
        AddNGram(new_tokens, 0, order_ + 1);
    }

    for (int j = 1; j < token_count - order_; j++) {
        AddNGram(new_tokens, j, j + order_);
    }

    // eos
    if (boundaries_) {
        AddNGram(new_tokens, token_count - order_ - 1, token_count);
    }
}

void NGramFeaturizer::LoadNGram(std::string& ngram_file) {
    std::ifstream f(ngram_file, std::ios::in);
    ScopeGuard guard([&]() { f.close(); });
    if (!f.is_open()) {
        PYIS_THROW("failed to open file %s", ngram_file.c_str());
    }
    std::string line;
    while (!f.eof()) {
        std::getline(f, line);
        rtrim_str(line);
        if (line.empty()) {
            continue;
        }

        auto end = line.find_first_of(" \t");
        if (end == std::string::npos) {
            PYIS_THROW("invalid ngram line: %s", line.c_str());
        }

        int id = std::stoi(line.substr(end + 1));
        AddNGram(line.substr(0, end), id);
    }
}

void NGramFeaturizer::DumpNGram(std::string& ngram_file) {
    auto ngrams = trie_.Items();
    std::ofstream f(ngram_file, std::ios::out);
    ScopeGuard guard([&]() { f.close(); });
    for (auto& i : ngrams) {
        std::string line = fmt_str("%s %d\n", std::get<0>(i).c_str(), std::get<1>(i));
        f.write(line.c_str(), line.length());
    }
}

std::vector<TextFeature> NGramFeaturizer::Transform(const std::vector<std::string>& tokens) const {
    std::vector<TextFeature> res;

    auto token_count = static_cast<int>(tokens.size());
    if (token_count < order_) {
        return res;
    }

    // compute sentence length
    int sentence_len = 0;
    for (int idx = 0; idx < token_count; idx++) {
        sentence_len += static_cast<int>(tokens[idx].length());
    }
    sentence_len += (token_count - 1);
    if (sentence_len <= 0) {
        return res;
    }

    std::vector<char> sentence(sentence_len + NGramFeaturizer::BOS_MARK.length() + NGramFeaturizer::EOS_MARK.length() +
                               3);
    std::vector<uint32_t> begin_indexes(token_count + 2);
    std::vector<uint32_t> end_indexes(token_count + 2);

    uint32_t cur = 0;
    memcpy(sentence.data(), NGramFeaturizer::BOS_MARK.data(), NGramFeaturizer::BOS_MARK.length());
    begin_indexes[0] = cur;
    end_indexes[0] = cur + static_cast<uint32_t>(NGramFeaturizer::BOS_MARK.length());
    cur += static_cast<uint32_t>(NGramFeaturizer::BOS_MARK.length());
    sentence[cur] = ' ';
    cur += 1;
    for (int idx = 0; idx < token_count; idx++) {
        const char* text = tokens[idx].data();
        auto len = static_cast<uint32_t>(tokens[idx].length());
        memcpy(sentence.data() + cur, text, len);
        begin_indexes[idx + 1] = cur;
        end_indexes[idx + 1] = cur + len;
        cur += len;
        sentence[cur] = ' ';
        cur += 1;
    }
    memcpy(sentence.data() + cur, NGramFeaturizer::EOS_MARK.data(), NGramFeaturizer::EOS_MARK.length());
    begin_indexes[token_count + 1] = cur;
    end_indexes[token_count + 1] = cur + static_cast<uint32_t>(NGramFeaturizer::EOS_MARK.length());
    cur += static_cast<uint32_t>(NGramFeaturizer::EOS_MARK.length());
    sentence[sentence.size() - 1] = '\0';

    // e.g. query:a b c, order:2, this is to check "BeginningOfDoc a b"
    if (boundaries_) {
        char* begin = sentence.data() + begin_indexes[0];
        char* end = sentence.data() + end_indexes[order_];
        (*end) = '\0';
        auto id = trie_.Lookup(begin);
        if (id.has_value()) {
            res.emplace_back(id.value(), 1.0, 0, order_ - 1);
        }
        (*end) = ' ';
    }

    // this is to check "a b", "b c"
    for (int i = 0; i <= token_count - order_; i++) {
        char* begin = sentence.data() + begin_indexes[i + 1];
        char* end = sentence.data() + end_indexes[i + order_];
        (*end) = '\0';
        auto id = trie_.Lookup(begin);
        if (id.has_value()) {
            res.emplace_back(id.value(), 1.0, i, i + order_ - 1);
        }
        (*end) = ' ';
    }

    // this is to check "b c EndOfDoc"
    if (boundaries_) {
        char* begin = sentence.data() + begin_indexes[token_count - order_ + 1];
        char* end = sentence.data() + end_indexes[end_indexes.size() - 1];
        (*end) = '\0';
        auto id = trie_.Lookup(begin);
        if (id.has_value()) {
            res.emplace_back(id.value(), 1.0, token_count - order_, token_count - 1);
        }
    }

    return res;
}

void NGramFeaturizer::AddNGram(std::vector<std::string>& tokens, int begin, int end) {
    std::ostringstream oss;

    oss << tokens[begin];
    for (int i = begin + 1; i < end; i++) {
        oss << ' ' << tokens[i];
    }

    auto ngram = oss.str();
    if (trie_.Lookup(ngram).has_error()) {
        trie_.Insert(ngram, next_id_);
        next_id_++;
    }
}

void NGramFeaturizer::AddNGram(const std::string& ngram, uint32_t id) {
    if (id < 0) {
        PYIS_THROW("invalid ngram id %d, must be a positive int32_t", id);
    }

    if (trie_.Lookup(ngram).has_error()) {
        trie_.Insert(ngram, id);
        if (id >= next_id_) {
            next_id_ = id + 1;
        }
    }
}

std::string NGramFeaturizer::Serialize(ModelStorage& storage) {
    std::string vocab_bin = storage.uniq_file(fmt_str("%dgram", order_), ".vocab.bin");
    auto ostream = storage.open_ostream(vocab_bin);
    trie_.Save(ostream);

    JsonPersistHelper jph(1);
    jph.add("order", order_);
    jph.add("boundaries", boundaries_);
    jph.add("next_id", next_id_);
    jph.add("vocab_file", vocab_bin);

    return jph.serialize();
}

void NGramFeaturizer::Deserialize(const std::string& state, ModelStorage& storage) {
    JsonPersistHelper jph(state);
    int version = jph.version();

    if (1 == version) {
        order_ = jph.get<int>("order");
        boundaries_ = jph.get<bool>("boundaries");
        next_id_ = jph.get<int32_t>("next_id");
        std::string vocab_bin = jph.get_file("vocab_file");
        auto istream = storage.open_istream(vocab_bin);
        trie_.Load(istream);
    } else {
        PYIS_THROW("NGramFeaturizer v%d is incompatible with the runtime", version);
    }
}

}  // namespace ops
}  // namespace pyis
