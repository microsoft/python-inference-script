#include <functional>
#include <iostream>
#include <regex>

#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/istreamwrapper.h"
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

class TokenWithRegularExp {
  public:
    void Set(std::string val) { m_text = ustring(val); }

    std::pair<bool, std::string> GetNextToken() {
        while (!m_text.empty()) {
            auto res = TryMatch();
            if (res.empty()) {
                m_text = ustring(m_text.substr(1));
                continue;
            }
            return {true, res};
        }
        
        return {false, ""};
    }

  private:
    std::string TryMatch() {
        if ((m_text[0] == U'\'') && (m_text.size() > 1)) {
            if ((m_text[1] == U's') || (m_text[1] == U't') || (m_text[1] == U'm') || (m_text[1] == U'd')) {
                ustring res = ustring(m_text.substr(0, 2));
                m_text = ustring(m_text.substr(2));
                return std::string(res);
            }

            if (m_text.size() > 2) {
                if (((m_text[1] == U'r') && (m_text[2] == U'e')) || ((m_text[1] == U'v') && (m_text[2] == U'e')) ||
                    ((m_text[1] == U'l') && (m_text[2] == U'l'))) {
                    ustring res = ustring(m_text.substr(0, 3));
                    m_text = ustring(m_text.substr(3));
                    return std::string(res);
                }
            }
        }

        // ?\p{L}+
        if ((m_text[0] == U' ') && (m_text.size() > 1) &&
            is_unicode_category_L(m_text[1])) {
            size_t i = 2;
            for (; i < m_text.size(); ++i) {
                if (!is_unicode_category_L(m_text[i])) break;
            }
            ustring res = ustring(m_text.substr(0, i));
            m_text = ustring(m_text.substr(i));
            return std::string(res);
        }
        if (is_unicode_category_L(m_text[0])) {
            size_t i = 1;
            for (; i < m_text.size(); ++i) {
                if (!is_unicode_category_L(m_text[i])) break;
            }
            ustring res = ustring(m_text.substr(0, i));
            m_text = ustring(m_text.substr(i));
            return std::string(res);
        }

        // ?\p{N}+
        if ((m_text[0] == U' ') && (m_text.size() > 1) &&
            is_unicode_category_N(m_text[1])) {
            size_t i = 2;
            for (; i < m_text.size(); ++i) {
                if (!is_unicode_category_N(m_text[i])) break;
            }
            ustring res = ustring(m_text.substr(0, i));
            m_text = ustring(m_text.substr(i));
            return std::string(res);
        }
        if (is_unicode_category_N(m_text[0])) {
            size_t i = 1;
            for (; i < m_text.size(); ++i) {
                if (!is_unicode_category_N(m_text[i])) break;
            }
            ustring res = ustring(m_text.substr(0, i));
            m_text = ustring(m_text.substr(i));
            return std::string(res);
        }

        // ?[^\s\p{L}\p{N}]+
        if ((m_text[0] == U' ') && (m_text.size() > 1) && (not_category_LNZ(m_text[1]))) {
            size_t i = 2;
            for (; i < m_text.size(); ++i) {
                if (!not_category_LNZ(m_text[i])) break;
            }
            ustring res = ustring(m_text.substr(0, i));
            m_text = ustring(m_text.substr(i));
            return std::string(res);
        }
        if (not_category_LNZ(m_text[0])) {
            size_t i = 1;
            for (; i < m_text.size(); ++i) {
                if (!not_category_LNZ(m_text[i])) break;
            }
            ustring res = ustring(m_text.substr(0, i));
            m_text = ustring(m_text.substr(i));
            return std::string(res);
        }

        // \s+(?!\S)|\s+
        if ((m_text.size() >= 1) && (is_unicode_category_Z(m_text[0]))) {
            size_t i = 1;
            for (; i < m_text.size(); ++i) {
                if (!is_unicode_category_Z(m_text[i])) break;
            }
            if ((i > 1) && (i != m_text.size()))  //\s+(?!\S)
            {
                i--;
                ustring res = ustring(m_text.substr(0, i));
                m_text = ustring(m_text.substr(i));
                return std::string(res);
            }
            // \s+
            ustring res = ustring(m_text.substr(0, i));
            m_text =ustring( m_text.substr(i));
            return std::string(res);
        }

        return "";
    }

  private:
    ustring m_text;
};

class GPT2Tokenizer : public Tokenizer {
  public:
    std::list<int> byte_list_;
    std::list<SpecialTokenInfo> token_list_;
    std::unordered_map<std::string, int> token_map_;
    void Add(std::string p_str, int p_id);
    
    std::vector<std::string> Tokenize(const std::string& input);
    
    std::list<std::pair<std::string, int>> SplitBySpeicalTokens(std::string input) const;
    void Load(std::istream& vocab_stream, std::istream& merges_stream, const std::string& unk_token,
              const std::string& special_tokens);

    GPT2Tokenizer(std::string vocab_file, const std::string& merges_file, const std::string& unk_token = "<|endoftext|>", const std::string& bos_token = "<|endoftext|>",
                  const std::string& eos_token = "<|endoftext|>", bool add_prefix_space = false);
  private:
    struct hash_pair {
        template <class T1, class T2>
        size_t operator()(const std::pair<T1, T2>& p) const {
            auto hash1 = std::hash<T1>{}(p.first);
            auto hash2 = std::hash<T2>{}(p.second);
            return hash1 ^ (hash2 << 16);
        }
    };
    std::unordered_map<std::pair<int, int>, BpeNode, hash_pair> bpe_map_;

    int byte_encoder_[256] = {};

    int unk_id_;

    void LoadVocabFile() override;

    
    void bpe(std::list<int>& vals) const;


  private:
    std::string merges_file_;
};
}  // namespace ops
}  // namespace pyis