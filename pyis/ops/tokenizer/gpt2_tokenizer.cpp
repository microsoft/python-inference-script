#include "gpt2_tokenizer.h"

namespace pyis {
namespace ops {

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
        if ((m_text[0] == U' ') && (m_text.size() > 1) && is_unicode_category_L(m_text[1])) {
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
        if ((m_text[0] == U' ') && (m_text.size() > 1) && is_unicode_category_N(m_text[1])) {
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
            m_text = ustring(m_text.substr(i));
            return std::string(res);
        }

        return "";
    }
    ustring m_text;
};

void GPT2Tokenizer::Add(std::string p_str, int p_id) {
    auto it = token_map_.find(p_str);
    if (it != token_map_.end()) {
        if (it->second != p_id) {
            PYIS_THROW("Duplicate special tokens.");
        }
    } else {
        token_map_[p_str] = p_id;
        token_list_.push_back(SpecialTokenInfo(std::move(p_str), p_id));
    }
}

inline std::vector<std::string> GPT2Tokenizer::Tokenize(const std::string& input) {
    std::vector<std::string> res;

    if (std::all_of(input.begin(), input.end(), isblank)) {
        return res;
    }

    auto special_token_split_res = SplitBySpeicalTokens(input);

    for (auto& seg_id : special_token_split_res) {

        if (seg_id.second != -1) {
            res.push_back(seg_id.first);
            continue;
        }

        auto cur_input = std::move(seg_id.first);
        TokenWithRegularExp reg;
        reg.Set(cur_input);

        while (true) {
            auto token = reg.GetNextToken();
            auto b = token.first;
            auto tok = token.second;
            if (!b) break;

            std::string utf8_token = std::string(tok);

            byte_list_.clear();
            for (char& cp : utf8_token) {
                byte_list_.push_back(byte_encoder_[static_cast<unsigned char>(cp)]);
            }

            bpe(byte_list_);

            for (auto p : byte_list_) {
                res.push_back(ConvertIdToToken(p));
            }
        }
    }
    return res;
}

inline std::list<std::pair<std::string, int>> GPT2Tokenizer::SplitBySpeicalTokens(std::string input) const {
    std::list<std::pair<std::string, int>> res;
    res.emplace_back(std::move(input), -1);
    for (const auto& st : token_list_) {
        std::list<std::pair<std::string, int>> new_split_res;
        for (auto& str : res) {
            if (str.second != -1) {
                new_split_res.push_back(std::move(str));
                continue;
            }
            auto it = str.first.begin();
            size_t search_pos = 0;
            while (it != str.first.end()) {
                auto search_it = std::search(it, str.first.end(), st.str.begin(), st.str.end());
                if (search_it == str.first.end()) {
                    new_split_res.emplace_back(str.first.substr(search_pos), -1);
                    break;
                }
                auto prefixLen = search_it - it;
                if (prefixLen != 0) {
                    new_split_res.emplace_back(str.first.substr(search_pos, prefixLen), -1);
                    search_pos += prefixLen;
                }
                new_split_res.emplace_back(str.first.substr(search_pos, st.str.size()), st.id);
                it = search_it + st.str.size();
                search_pos += st.str.size();
            }
        }
        std::swap(new_split_res, res);
    }
    return res;
}

inline void GPT2Tokenizer::Load(std::istream& vocab_stream, std::istream& merges_stream, const std::string& unk_token) {
    rapidjson::Document tok_json;
    rapidjson::IStreamWrapper vocab_wrapper(vocab_stream);
    tok_json.ParseStream(vocab_wrapper);
    vocab_map_.clear();
    for (auto ite = tok_json.MemberBegin(); ite != tok_json.MemberEnd(); ++ite) {
        vocab_map_.insert({ite->name.GetString(), ite->value.GetInt()});
    }

    auto it = vocab_map_.find(unk_token);
    if (it == vocab_map_.end()) {
        int id = static_cast<int>(vocab_map_.size());
        vocab_map_[unk_token] = id;
    }

    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> str_convert;
    for (auto i = 33; i <= 126; ++i) {
        byte_encoder_[i] = ConvertTokenToId(str_convert.to_bytes(static_cast<char32_t>(i)));
    }
    for (auto i = 161; i <= 172; ++i) {
        byte_encoder_[i] = ConvertTokenToId(str_convert.to_bytes(static_cast<char32_t>(i)));
    }
    for (auto i = 174; i <= 255; ++i) {
        byte_encoder_[i] = ConvertTokenToId(str_convert.to_bytes(static_cast<char32_t>(i)));
    }

    int index = 256;
    for (auto i = 0; i < 33; ++i) {
        byte_encoder_[i] = ConvertTokenToId(str_convert.to_bytes(static_cast<char32_t>(index)));
        ++index;
    }
    for (auto i = 127; i < 161; ++i) {
        byte_encoder_[i] = ConvertTokenToId(str_convert.to_bytes(static_cast<char32_t>(index)));
        ++index;
    }
    byte_encoder_[173] = ConvertTokenToId(str_convert.to_bytes(static_cast<char32_t>(index)));
    ++index;

    index = 0;
    std::string line;
    while (std::getline(merges_stream, line)) {
        line = std::regex_replace(line, std::regex("\r"), "");
        if (line.empty()) continue;
        if ((line[0] == '#') && (index == 0)) continue;
        auto pos = line.find(' ');
        if (pos == std::string::npos) {
            PYIS_THROW("Cannot parse line: ");
        }
        std::string w1 = line.substr(0, pos);
        std::string w2 = line.substr(pos + 1);
        int iw1 = ConvertTokenToId(w1);
        int iw2 = ConvertTokenToId(w2);
        int iww = ConvertTokenToId(w1 + w2);
        std::pair<int, int> key{iw1, iw2};
        BpeNode value{iww, index++};
        bpe_map_[key] = value;
    }

    vocab_map_reverse_.clear();
    for (const auto& ite : vocab_map_) {
        vocab_map_reverse_[ite.second] = ite.first;
    }
}

GPT2Tokenizer::GPT2Tokenizer(std::string vocab_file, const std::string& merges_file, const std::string& unk_token,
                             const std::string& bos_token, const std::string& eos_token, bool add_prefix_space)
    : Tokenizer(vocab_file), merges_file_(merges_file) {
    LoadVocabFile();
}

void GPT2Tokenizer::LoadVocabFile() {
    std::ifstream vocab_stream(vocab_file_);
    std::ifstream merges_stream(merges_file_);
    Load(vocab_stream, merges_stream, unk_token_);
}

void GPT2Tokenizer::bpe(std::list<int>& vals) const {
    while (vals.size() >= 2) {
        auto pos_it = vals.end();
        int minval = std::numeric_limits<int>::max();
        int ori_id1 = 0, ori_id2 = 0;
        int aim_id = 0;
        for (auto it = vals.begin(); it != vals.end(); ++it) {
            auto it2 = it;
            ++it2;
            if (it2 == vals.end()) break;
            auto map_it = bpe_map_.find({*it, *it2});
            if (map_it == bpe_map_.end()) continue;
            if (minval > map_it->second.value) {
                ori_id1 = *it;
                ori_id2 = *it2;
                minval = map_it->second.value;
                pos_it = it;
                aim_id = map_it->second.id;
            }
        }
        if (pos_it == vals.end()) break;

        pos_it = vals.erase(pos_it);
        *pos_it = aim_id;
        for (++pos_it; pos_it != vals.end(); ++pos_it) {
            if (*pos_it != ori_id1) continue;
            auto it2 = pos_it;
            ++it2;
            if (it2 == vals.end()) break;
            if (*it2 != ori_id2) continue;
            pos_it = vals.erase(pos_it);
            *pos_it = aim_id;
        }
    }
}

}  // namespace ops
}  // namespace pyis
