#include <functional>
#include <iostream>
#include <list>
#include <regex>

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/rapidjson.h"
#include "tokenizer_base.h"

namespace pyis {
namespace ops {

struct BpeNode {
    int id;
    int value;
};

struct SpecialTokenInfo {
    std::string str;
    int id;
    SpecialTokenInfo(std::string p_str, int p_id) : str(std::move(p_str)), id(p_id) {
        if (str.empty()) {
            PYIS_THROW("Empty special token.");
        }
    }
};

class TokenWithRegularExp;

class GPT2Tokenizer : public Tokenizer {
  public:
    void Add(std::string p_str, int p_id);
    std::vector<std::string> Tokenize(const std::string& input);
    std::list<std::pair<std::string, int>> SplitBySpeicalTokens(std::string input) const;
    void Load(std::istream& vocab_stream, std::istream& merges_stream, const std::string& unk_token);
    GPT2Tokenizer(std::string vocab_file, const std::string& merges_file,
                  const std::string& unk_token = "<|endoftext|>", const std::string& bos_token = "<|endoftext|>",
                  const std::string& eos_token = "<|endoftext|>", bool add_prefix_space = false);

    std::vector<int64_t> AddSpecialToken(const std::vector<int64_t>& code) override;
    std::vector<int64_t> AddSpecialToken(const std::vector<int64_t>& ids1, const std::vector<int64_t>& ids2) override;

  private:
    struct hash_pair {
        template <class T1, class T2>
        size_t operator()(const std::pair<T1, T2>& p) const {
            auto hash1 = std::hash<T1>{}(p.first);
            auto hash2 = std::hash<T2>{}(p.second);
            return hash1 ^ (hash2 << 16);
        }
    };

    std::list<int> byte_list_;
    std::list<SpecialTokenInfo> token_list_;
    std::unordered_map<std::string, int> token_map_;
    std::unordered_map<std::pair<int, int>, BpeNode, hash_pair> bpe_map_;
    std::string merges_file_;
    int byte_encoder_[256] = {};

    void LoadVocabFile() override;
    void bpe(std::list<int>& vals) const;
};

}  // namespace ops
}  // namespace pyis