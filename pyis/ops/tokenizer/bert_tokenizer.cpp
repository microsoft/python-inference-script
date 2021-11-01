// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "bert_tokenizer.h"

#include <regex>
#include <string>

#include "pyis/share/str_utils.h"

namespace pyis {
namespace ops {

BertTokenizer::BertTokenizer()
    : do_lower_case_(false), do_basic_tokenize_(false), tokenize_chinese_chars_(false), strip_accents_(false) {}

BertTokenizer::BertTokenizer(const std::string& vocab_file, bool do_lower_case, bool do_basic_tokenize,
                             const std::string& cls_token, const std::string& sep_token, const std::string& unk_token,
                             const std::string& pad_token, const std::string& mask_token, bool tokenize_chinese_chars,
                             bool strip_accents, const std::string& suffix_indicator)
    : Tokenizer(vocab_file, cls_token, sep_token, unk_token, pad_token, mask_token),
      do_lower_case_(do_lower_case),
      do_basic_tokenize_(do_basic_tokenize),
      tokenize_chinese_chars_(tokenize_chinese_chars),
      strip_accents_(strip_accents),
      suffix_indicator_(suffix_indicator) {
    if (do_basic_tokenize) {
        basic_tokenizer_ =
            std::make_shared<BasicTokenizer>(do_lower_case, do_basic_tokenize, strip_accents, true, true);
    }
    wordpiece_tokenizer_ = std::make_shared<WordpieceTokenizer>(vocab_file, cls_token, sep_token, unk_token, pad_token,
                                                                mask_token, suffix_indicator);
}

std::vector<std::string> BertTokenizer::Tokenize(const std::string& str) {
    if (do_basic_tokenize_) {
        return wordpiece_tokenizer_->Tokenize(basic_tokenizer_->Tokenize(str));
    }
    return wordpiece_tokenizer_->Tokenize(str);
}

std::string BertTokenizer::Serialize(ModelStorage& fs) {
    std::string vocab_path = fs.uniq_file("bert_tokenizer", ".vocab.txt");
    fs.add_file(vocab_file_, vocab_path);
    std::string config_file = fs.uniq_file("bert_tokenizer", ".config.json");

    JsonPersistHelper jph(1);
    jph.add_file("vocab_file", vocab_path);
    jph.add("start_token", cls_token_);
    jph.add("end_token", sep_token_);
    jph.add("unk_token", unk_token_);
    jph.add("pad_token", pad_token_);
    jph.add("mask_token", mask_token_);
    jph.add("do_lower_case", do_lower_case_);
    jph.add("do_basic_tokenize", do_basic_tokenize_);
    jph.add("tokenize_chinese_chars", tokenize_chinese_chars_);
    jph.add("strip_accents", strip_accents_);
    jph.add("suffix_indicator", suffix_indicator_);

    std::string state = jph.serialize(config_file, fs);
    return state;
}

void BertTokenizer::Deserialize(const std::string& state, ModelStorage& fs) {
    JsonPersistHelper jph(state, fs);
    int version = jph.version();
    if (1 == version) {
        vocab_file_ = FileSystem::join_path({fs.root_dir(), jph.get_file("vocab_file")});
        cls_token_ = jph.get("start_token");
        sep_token_ = jph.get("end_token");
        unk_token_ = jph.get("unk_token");
        pad_token_ = jph.get("pad_token");
        mask_token_ = jph.get("mask_token");
        do_lower_case_ = jph.get<bool>("do_lower_case");
        do_basic_tokenize_ = jph.get<bool>("do_basic_tokenize");
        tokenize_chinese_chars_ = jph.get<bool>("tokenize_chinese_chars");
        strip_accents_ = jph.get<bool>("strip_accents");
        suffix_indicator_ = jph.get("suffix_indicator");
        LoadVocabFile();
        if (do_basic_tokenize_) {
            basic_tokenizer_ =
                std::make_shared<BasicTokenizer>(do_lower_case_, do_basic_tokenize_, strip_accents_, true, true);
        }
        wordpiece_tokenizer_ = std::make_shared<WordpieceTokenizer>(vocab_file_, cls_token_, sep_token_, unk_token_,
                                                                    pad_token_, mask_token_, suffix_indicator_);
    } else {
        PYIS_THROW(fmt_str("BertTokenizer v{} is incompatible with the runtime", version).c_str());
    }
}

std::string BertTokenizer::Decode(const std::vector<int64_t>& code, bool skip_special_tokens,
                                  bool clean_up_tokenization_spaces) {
    std::vector<std::string> sub_texts;
    std::set<std::string> special_tokens({unk_token_, pad_token_, cls_token_, mask_token_, sep_token_});
    for (const auto& id : code) {
        std::string token = ConvertIdToToken(id);
        if (!skip_special_tokens || special_tokens.count(token) == 0U) {
            if (!sub_texts.empty() && token.length() > suffix_indicator_.length() &&
                token.substr(0, suffix_indicator_.length()) == suffix_indicator_) {
                token = token.substr(suffix_indicator_.length());
                sub_texts.back().append(token);
            } else {
                sub_texts.emplace_back(token);
            }
        }
    }
    std::string text = join_str(sub_texts, " ");
    if (clean_up_tokenization_spaces) {
        CleanUpTokenization(text);
    }
    return text;
}

}  // namespace ops
}  // namespace pyis
