// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "pyis/ops/tokenizer/tokenizer_base.h"

namespace pyis {
namespace ops {

class WordpieceTokenizer : public Tokenizer {
  public:
    explicit WordpieceTokenizer(const std::string& vocab_file, const std::string& cls_token = "[CLS]",
                                const std::string& sep_token = "[SEP]", const std::string& unk_token = "[UNK]",
                                const std::string& pad_token = "[PAD]", const std::string& mask_token = "[MASK]",
                                std::string suffix_indicator = "##");
    std::vector<std::string> Tokenize(const std::string& str) override;

    std::vector<std::string> Tokenize(const std::vector<std::string>& tokens);

  private:
    std::string word_piece_prefix_;
    void GreedySearch(const std::string& token, std::vector<std::string>& tokenized_result);
};
}  // namespace ops
}  // namespace pyis