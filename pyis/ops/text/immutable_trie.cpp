// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "pyis/ops/text/immutable_trie.h"

namespace pyis {
namespace ops {

constexpr int BinaryWriter::BUFFER_LEN;
constexpr int ImmutableTrie::TRANSLATE_TABLE_SIZE;

const uint8_t ImmutableTrie::BYTE_NO_MATCH;
const uint8_t ImmutableTrie::BYTE_SEPERATOR;
const uint8_t ImmutableTrie::BYTE_ANY_WORD;
const uint8_t ImmutableTrie::BYTE_ANY_CHAR;

const uint8_t ImmutableTrie::BYTE_EDGE_TABLE;
const uint8_t ImmutableTrie::BYTE_EDGE_LIST;
const uint8_t ImmutableTrie::BYTE_SINGLE_FOLLOW_MAX;
const uint8_t ImmutableTrie::BYTE_SINGLE_FOLLOW_INTERNAL_MORE;
const uint8_t ImmutableTrie::BYTE_SINGLE_FOLLOW_LEAF_MORE;
const uint8_t ImmutableTrie::BYTE_SINGLE_FOLLOW_MIN;

const uint8_t ImmutableTrie::BYTE_MAGIC_EDGE_LIST;
const uint8_t ImmutableTrie::BYTE_MAGIC_EDGE_TABLE;

const int32_t ImmutableTrie::NO_MATCH;
const int32_t ImmutableTrie::MATCH_LEAF;
const int32_t ImmutableTrie::MATCH_INTERNAL;

size_t BinaryReader::ReadBuffer() {
    if (!ifs_->good()) {
        return 0;
    }
    ifs_->read(reinterpret_cast<char*>(buffer_), 256);
    return ifs_->gcount();
}

Expected<void> BinaryReader::DecodeUInt32(uint32_t* ret_value) {
    uint8_t nibble;
    uint32_t result = 0;
    do {
        if (read_cnt_ >= capacity_) {
            read_cnt_ = 0;
            capacity_ = ReadBuffer();
            if (capacity_ <= 0) {
                return Expected<void>(std::runtime_error("ReadBuffer returned 0"));
            }
            lower_nibble_ = true;
        }
        if (lower_nibble_) {
            nibble = (buffer_[read_cnt_] & 0x0F);
        } else {
            nibble = (buffer_[read_cnt_] >> 4);
            read_cnt_++;
        }
        lower_nibble_ = !lower_nibble_;
        result = (result << 3) | (nibble & 0x07);
    } while ((nibble & 0x08) == 0);
    *ret_value = result;
    return Expected<void>();
}

Expected<void> BinaryReader::ReadTaggedValue(uint32_t tag, uint32_t* ret_value) {
#ifdef VALIDATE_TAGS
    uint32_t stream_tag;
    *ret_value = 0;
    auto hr = DecodeUInt32(&stream_tag);
    if (hr.has_error()) {
        return hr;
    }
    if (stream_tag != tag) {
        return Expected<void>(
            std::runtime_error(pyis::fmt_str("stream tag mismatch: expected %u, got %u", tag, stream_tag)));
    }
#endif
    return DecodeUInt32(ret_value);
}

Expected<void> BinaryReader::ReadInt16(uint32_t tag, int16_t* ret_value) {
    uint32_t ival;
    auto result = ReadTaggedValue(tag, &ival);
    if (!result.has_error()) {
        *ret_value = static_cast<int16_t>(ival);
    }
    return result;
}

Expected<void> BinaryReader::ReadUInt32(uint32_t tag, uint32_t* ret_value) { return ReadTaggedValue(tag, ret_value); }

Expected<void> BinaryReader::ReadInt32(uint32_t tag, int32_t* ret_value) {
    return ReadTaggedValue(tag, reinterpret_cast<uint32_t*>(ret_value));
}

Expected<void> BinaryReader::ReadBuffer(uint32_t tag, uint8_t* buffer, uint32_t buffer_size) {
#ifdef VALIDATE_TAGS
    {
        uint32_t stream_tag;
        auto result = DecodeUInt32(&stream_tag);
        if (result.has_error()) {
            return result;
        }
        if (stream_tag != tag) {
            return Expected<void>(std::runtime_error("Stream Tag Mismatch"));
        }
    }
#endif
    uint32_t buffer_length;
    auto result = DecodeUInt32(&buffer_length);
    if (result.has_error()) {
        return result;
    }
    if (buffer_length != buffer_size) {
        return Expected<void>(std::runtime_error("Buffer Size Mismatch"));
    }
    uint32_t loop_length = buffer_size & ~0x03;
#ifdef ROUND_UP_BUFFER_SIZE
    if (loop_length != buffer_length) {
        loop_length += 4;
    }
    for (uint32_t pos = 0; pos < loopLen; pos += 4, buffer += 4) {
        if (DecodeUInt32(reinterpret_cast<uint32_t*>(buffer)).has_error()) {
            break;
        }
    }
#else
    for (uint32_t pos = 0; pos < loop_length; pos += 4, buffer += 4) {
        if (DecodeUInt32(reinterpret_cast<uint32_t*>(buffer)).has_error()) {
            break;
        }
    }
    uint32_t remaining_cnt = buffer_size & 0x03;
    if (remaining_cnt > 0) {
        uint8_t* buf = buffer;
        uint32_t remaining;

        auto result = DecodeUInt32(&remaining);
        if (result.has_error()) {
            return result;
        }
        for (uint32_t pos = 0; pos < remaining_cnt; ++pos, ++buf) {
            *buf = static_cast<uint8_t>(remaining & 0xFF);
            remaining >>= 8;
        }
    }
#endif
    return Expected<void>();
}

BinaryReader::BinaryReader(const std::string& path) : read_cnt_(0), capacity_(0), lower_nibble_(true), buffer_{0} {
    file_buffer_ = new std::filebuf();
    if (file_buffer_->open(path, std::ios::in | std::ios::binary) == nullptr) {
        delete file_buffer_;
        file_buffer_ = nullptr;
        PYIS_THROW("Cannot open file %s", path.c_str());
    }
    ifs_ = std::make_shared<std::istream>(file_buffer_);
}

BinaryReader::BinaryReader(std::shared_ptr<std::istream> ptr) {
    ifs_ = std::move(ptr);
    file_buffer_ = nullptr;
}

BinaryReader::~BinaryReader() {
    if (file_buffer_ != nullptr) {
        file_buffer_->close();
        delete file_buffer_;
    }
    ifs_.reset();
}

void ImmutableTrieNode::AddChild(const std::string& str, int idx, uint32_t data) {
    char ch = str[idx];
    auto find_char = [ch](ImmutableTrieNode* node) { return node->conv_char_ == ch; };
    size_t child_index = std::find_if(children_.begin(), children_.end(), find_char) - children_.begin();
    if (child_index == children_.size()) {
        children_.emplace_back(new ImmutableTrieNode(ch));
    }

    if (idx < str.length() - 1) {
        children_[child_index]->AddChild(str, idx + 1, data);
    } else {
        children_[child_index]->SetData(data);
    }
}

void ImmutableTrieNode::SetData(uint32_t data) {
    has_data_ = true;
    data_ = data;
}

ImmutableTrieNode::ImmutableTrieNode() {
    has_data_ = false;
    data_ = 0;
    conv_char_ = 0;
}

ImmutableTrieNode::ImmutableTrieNode(char conv_char) {
    has_data_ = false;
    data_ = 0;
    conv_char_ = conv_char;
}

BinaryWriter::BinaryWriter(const std::string& path) : used_(0), buffer_{0} {
    file_buffer_ = new std::filebuf();
    if (file_buffer_->open(path, std::ios::out | std::ios::binary) == nullptr) {
        delete file_buffer_;
        file_buffer_ = nullptr;
        PYIS_THROW("Cannot open file %s", path.c_str());
    }
    ofs_ = std::make_shared<std::ostream>(file_buffer_);
}

BinaryWriter::BinaryWriter(std::shared_ptr<std::ostream> ptr) : used_(0), buffer_{0} {
    ofs_ = std::move(ptr);
    file_buffer_ = nullptr;
}

BinaryWriter::~BinaryWriter() {
    if (used_ > 0) {
        WriteBuffer((used_ + 1) / 2);
    }
    if (file_buffer_ != nullptr) {
        file_buffer_->close();
        delete file_buffer_;
    }
    ofs_.reset();
}

void BinaryWriter::SetNibble(uint8_t* buffer_array, int i, uint8_t n) {
    if ((i & 1) == 0) {
        buffer_array[i / 2] = (buffer_array[i / 2] & 0xF0) | (n & 0x0F);
    } else {
        buffer_array[i / 2] = (buffer_array[i / 2] & 0x0F) | (n & 0x0F) << 4;
    }
}

void BinaryWriter::WriteBuffer(const int& count) { ofs_->write(reinterpret_cast<const char*>(buffer_), count); }

void BinaryWriter::WriteNibble(uint8_t nibble) {
    SetNibble(buffer_, used_++, nibble);
    if (used_ / 2 >= BUFFER_LEN) {
        WriteBuffer(BUFFER_LEN);
        used_ = 0;
    }
}

//
// EncodeUINT encodes integer value as a serias of nibbles (nibble = half a byte).
// 1. The integer value is split into groups of 3 bits each.
// 2. Leading groups of zeroes are discarded (except for the last one, which can happen if the value is
// zero).
// 3. The rest are emitted as series of nibbles, the high bit of the nibble indicating whether it is
//    the last one in the series.
//
// Examples: (here the values consist of 8 bits, but the same procedure applies to 32 bit ints).
//   value     3-bit groups   encoded series
//  00000000 -> 000 000 000 -> 1000
//  00000001 -> 000 000 001 -> 1001
//  00100110 -> 000 100 110 -> 0100 1110
//  10100110 -> 010 100 110 -> 0010 0100 1110
//  11111111 -> 011 111 111 -> 0011 0111 1111
//              ^------- the first group is padded with zero(es).
//
// In order to achieve good compression using this algorithm,
// the majority of encoded values should be of small magnitude.
//
void BinaryWriter::EncodeUINT(uint32_t data) {
    int n;
    for (n = 10; n > 0; n--) {
        if ((data >> n * 3) != 0) {
            break;
        }
    }
    for (/**/; n > 0; n--) {
        // Write data out 3 bits at a time
        WriteNibble(static_cast<uint8_t>((data >> n * 3) & 0x7));
    }

    WriteNibble(static_cast<uint8_t>(
        (data & 0x7) | 0x8));  // Write out the last nibble setting high bit to 1 to indicate end of the series.
}

void BinaryWriter::WriteTaggedValue(uint32_t tag, uint32_t value) {
#ifdef VALIDATE_TAGS
    EncodeUINT(tag);
#endif
    EncodeUINT(value);
}

void BinaryWriter::WriteBool(uint32_t tag, bool b) { WriteTaggedValue(tag, static_cast<uint32_t>(b)); }

void BinaryWriter::WriteInt16(uint32_t tag, int16_t s) { WriteTaggedValue(tag, static_cast<uint32_t>(s)); }

void BinaryWriter::WriteInt32(uint32_t tag, int32_t s) { WriteTaggedValue(tag, static_cast<uint32_t>(s)); }

void BinaryWriter::WriteUInt32(uint32_t tag, uint32_t s) { WriteTaggedValue(tag, s); }

void BinaryWriter::WriteBuffer(uint32_t tag, const uint8_t* buffer, uint32_t buffer_length) {
#ifdef VALIDATE_TAGS
    EncodeUINT(tag);
#endif
    EncodeUINT(buffer_length);
    int loop_cnt = buffer_length & ~0x03;
#ifdef ROUND_UP_BUFFER_SIZE
    if (loop_cnt != bufferLen) loop_cnt = loop_cnt + 4;
    for (int pos = 0; pos < loop_cnt; pos += 4, buffer += 4) {
        EncodeUINT(*(UINT*)buffer);
    }
#else
    for (int pos = 0; pos < loop_cnt; pos += 4, buffer += 4) {
        EncodeUINT(*reinterpret_cast<uint32_t*>(const_cast<uint8_t*>(buffer)));
    }
    int remaining_cnt = buffer_length & 0x3;
    uint32_t remaining = 0;
    const auto* buf = buffer;
    for (int pos = 0; pos < remaining_cnt; ++pos, ++buf) {
        uint32_t next_val = *buf;
        remaining |= (next_val << (pos * 8));
    }
    if (remaining_cnt > 0) {
        EncodeUINT(remaining);
    }
#endif
}

ImmutableTrieConstructor::ImmutableTrieConstructor(const std::vector<std::tuple<std::string, uint32_t>>& data) {
    root_ = new ImmutableTrieNode();
    for (int i = 0; i < 256; i++) {
        translator_[i] = i;
        translation_[i] = i;
    }
    list_buffer_ = std::make_shared<std::vector<uint8_t>>();
    memset(hist_char_counts_single_follow_data_leaf_, 0, sizeof(hist_char_counts_single_follow_data_leaf_));
    memset(hist_char_counts_single_follow_data_internal_, 0, sizeof(hist_char_counts_single_follow_data_internal_));
    memset(hist_char_counts_, 0, sizeof(hist_char_counts_));
    uint32_t max_data = 0;

    for (const auto& i : data) {
        root_->AddChild(std::get<0>(i), 0, std::get<1>(i));
        for (const auto& c : std::get<0>(i)) {
            hist_char_counts_[c]++;
        }
        max_data = std::max(max_data, std::get<1>(i));
    }

    BuildHistChildrenCounts(root_);
    BuildTranslator();
    BuildSingleFollowTranslator();

    payload_size_ = ceil(log2(static_cast<int64_t>(max_data) + 1) + 7) / 8;
    EncodeTrieNode(root_);

    for (const auto& x : list_trie_data_) {
        trie_data_.insert(trie_data_.end(), x->begin(), x->end());
    }
    if (!list_buffer_->empty()) {
        trie_data_.insert(trie_data_.end(), list_buffer_->begin(), list_buffer_->end());
    }
}

ImmutableTrieConstructor::~ImmutableTrieConstructor() {
    CleanUp(root_);
    delete[] single_follow_enc_;
}

Expected<void> ImmutableTrieConstructor::WriteToFile(const std::string& path) {
    BinaryWriter writer(path);
    return WriteToFile(writer);
}

Expected<void> ImmutableTrieConstructor::WriteToFile(std::shared_ptr<std::ostream>& os) {
    BinaryWriter writer(os);
    return WriteToFile(writer);
}

Expected<void> ImmutableTrieConstructor::WriteToFile(BinaryWriter& bw) {
    bw.WriteInt32('MCHV', max_char_val_);
    bw.WriteInt32('CBPL', payload_size_);

    bw.WriteBuffer('TLTB', translation_, 256);
    if (single_follow_enc_ != nullptr) {
        bw.WriteInt32('TSTL', single_follow_dec_.size());
        for (const auto& i : single_follow_dec_) {
            bw.WriteInt16('TLSF', static_cast<const int16_t>(i));
        }
    }

    bw.WriteInt32('TREL', trie_data_.size());
    bw.WriteBuffer('TREN', trie_data_.data(), trie_data_.size());
    return Expected<void>();
}

void ImmutableTrieConstructor::WriteToTrie(ImmutableTrie& trie) {
    trie.max_char_val_ = max_char_val_;
    trie.payload_size_ = payload_size_;
    memcpy(trie.translate_, translation_, sizeof(trie.translate_));
    if (single_follow_enc_ != nullptr) {
        trie.size_single_follow_dec_ = single_follow_dec_.size();
        trie.single_follow_dec_ = new uint16_t[trie.size_single_follow_dec_ + 4];
        memcpy(trie.single_follow_dec_, single_follow_dec_.data(), sizeof(uint16_t) * trie.size_single_follow_dec_);
    }
    trie.trie_data_size_ = trie_data_.size();
    trie.trie_data_ = new uint8_t[trie.trie_data_size_ + 4];
    memcpy(trie.trie_data_, trie_data_.data(), sizeof(uint8_t) * trie.trie_data_size_);
    for (uint8_t& i : trie.detranslate_) {
        i = ' ';
    }
    for (size_t i = 0; i < ImmutableTrie::TRANSLATE_TABLE_SIZE; i++) {
        trie.detranslate_[trie.translate_[i]] = i;
    }

    trie.detranslate_[trie.translate_[' ']] = ' ';
    trie.detranslate_[ImmutableTrie::BYTE_NO_MATCH] = ' ';
}

void ImmutableTrieConstructor::BuildHistChildrenCounts(ImmutableTrieNode* node) {
    if (node == nullptr) {
        return;
    }
    if (node->children_.size() == 1) {
        if (node->children_[0]->has_data_) {
            if (node->children_[0]->children_.empty()) {
                hist_char_counts_single_follow_data_leaf_[node->children_[0]->conv_char_]++;
            } else {
                hist_char_counts_single_follow_data_internal_[node->children_[0]->conv_char_]++;
            }
        }
    }
    for (const auto& child : node->children_) {
        BuildHistChildrenCounts(child);
    }
}

void ImmutableTrieConstructor::BuildTranslator() {
    int trans_chars[256];
    memset(trans_chars, -1, sizeof(trans_chars));
    max_char_val_ = 3;
    for (int i = 0; i < 256; i++) {
        uint8_t ch = translator_[i];
        if (ch == TRIE_SEPERATOR_) {
            translation_[i] = BYTE_SEPERATOR_;
        } else if (ch == TRIE_ANY_WORD_) {
            translation_[i] = BYTE_ANY_WORD_;
        } else if (ch == TRIE_ANY_CHAR_) {
            translation_[i] = BYTE_ANY_CHAR_;
        } else if (hist_char_counts_[ch] == 0) {
            translation_[i] = BYTE_NO_MATCH_;
        } else if (trans_chars[ch] != -1) {
            translation_[i] = static_cast<uint8_t>(trans_chars[ch]);
        } else {
            translation_[i] = static_cast<uint8_t>(++max_char_val_);
        }
        if (trans_chars[ch] == -1) {
            trans_chars[ch] = translation_[i];
        } else {
            // assert transCh[ch] == translation[i]
        }
    }
}

void ImmutableTrieConstructor::BuildSingleFollowTranslator() {
    if ((max_char_val_ + 1) * 3 - 1 <= BYTE_SINGLE_FOLLOW_MAX_) {
        return;
    }
    std::vector<std::pair<int, uint16_t>> char_cnts_and_vals;
    char_cnts_and_vals.resize(512);
    for (int i = 0; i < 512; i++) {
        char_cnts_and_vals[i].second = i;
    }
    for (int i = 0; i < 256; i++) {
        char_cnts_and_vals[i].first = hist_char_counts_single_follow_data_internal_[i];
        char_cnts_and_vals[i + 256].first = hist_char_counts_single_follow_data_leaf_[i];
    }
    std::sort(char_cnts_and_vals.begin(), char_cnts_and_vals.end());

    single_follow_enc_ = new uint16_t[512];
    memset(single_follow_enc_, -1, sizeof(uint16_t) * 512);

    uint16_t single_follow_id = 0;

    for (int i = 511; i >= 0; --i) {
        if (char_cnts_and_vals[i].first <= 0) break;
        single_follow_enc_[char_cnts_and_vals[i].second] = single_follow_id++;
    }

    int max_size_decoded = BYTE_SINGLE_FOLLOW_MIN_ - max_char_val_ - 1;
    single_follow_dec_.resize(std::min(max_size_decoded, static_cast<int>(single_follow_id)));
    for (int i = 0; i < 512; i++) {
        uint16_t enc = single_follow_enc_[i];
        if (enc < single_follow_dec_.size()) {
            int char_value = (i < 256) ? translation_[i] : translation_[i - 256];
            int is_internal = (i < 256) ? 256 : 0;
            single_follow_dec_[enc] = static_cast<uint16_t>(char_value | is_internal);
        }
    }
}

int ImmutableTrieConstructor::EncodeTrieNode(ImmutableTrieNode* node) {
    int bytes_cnt;

    if (node->children_.empty()) {
        return 0;
    }

    bytes_cnt = EncodeTrieNodeSingleFollow(node);
    if (bytes_cnt >= 0) {
        return bytes_cnt;
    }

    bytes_cnt = EncodeTrieNodeEdgeList(node);
    if (bytes_cnt >= 0) {
        return bytes_cnt;
    }

    return EncodeTrieNodeEdgeTable(node);
}

void ImmutableTrieConstructor::EncodeChar(const uint8_t& ch) {
    uint8_t char_value = translation_[ch];
    list_buffer_->emplace_back(char_value);
}

int ImmutableTrieConstructor::EncodeData(const uint32_t& data) { return EncodeUINT(data, payload_size_); }

int ImmutableTrieConstructor::EncodeOffset(ImmutableTrieNode* node, const int& offset, const int& offset_size) {
    int is_internal = (node->children_.empty()) ? 0 : 1;
    int has_data = (node->has_data_) ? 2 : 0;

    int tag_offset = is_internal | has_data;
    int offset_encoded = offset << 2 | tag_offset;
    return EncodeUINT(static_cast<uint32_t>(offset_encoded), offset_size);
}

int ImmutableTrieConstructor::EncodeUINT(uint32_t x, int payload_size) {
    for (int i = 0; i < payload_size; i++) {
        list_buffer_->emplace_back(static_cast<uint8_t>(x & 0xff));
        x >>= 8;
    }
    return payload_size;
}

size_t ImmutableTrieConstructor::EncodeTrieNodeSingleFollow(ImmutableTrieNode* node) {
    if (node->children_.size() != 1) return -1;
    size_t byte_begin_pos = list_buffer_->size();
    int ch = static_cast<uint8_t>(node->children_[0]->conv_char_);
    if (!node->children_[0]->has_data_) {
        EncodeChar(static_cast<uint8_t>(ch));
    } else {
        bool is_leaf = (node->children_[0]->children_.empty());
        if (single_follow_enc_ == nullptr) {
            int char_encoded = translation_[ch];
            auto tag = static_cast<uint32_t>(is_leaf ? char_encoded + max_char_val_ + 1
                                                     : char_encoded + 2 * max_char_val_ + 2);
            list_buffer_->emplace_back(static_cast<uint8_t>(tag));
        } else {
            if (is_leaf) {
                ch += 256;
            }

            uint16_t char_encoded = single_follow_enc_[ch];
            if (char_encoded < single_follow_dec_.size()) {
                int tag = char_encoded + max_char_val_ + 1;
                list_buffer_->emplace_back(static_cast<uint8_t>(tag));
            } else {
                if (is_leaf) {
                    list_buffer_->emplace_back(BYTE_SINGLE_FOLLOW_LEAF_MORE_);
                } else {
                    list_buffer_->emplace_back(BYTE_SINGLE_FOLLOW_INTERNAL_MORE_);
                }
                EncodeChar(node->children_[0]->conv_char_);
            }
        }
        EncodeData(node->children_[0]->data_);
    }
    size_t bytes_cnt = list_buffer_->size() - byte_begin_pos;
    bytes_cnt += EncodeTrieNode(node->children_[0]);

    return bytes_cnt;
}

void ImmutableTrieConstructor::InsertByteList(const std::shared_ptr<std::vector<uint8_t>>& content) {
    if (!list_buffer_->empty()) {
        list_trie_data_.emplace_back(list_buffer_);
    }
    list_trie_data_.emplace_back(content);
    list_buffer_ = std::make_shared<std::vector<uint8_t>>();
}

int ImmutableTrieConstructor::EncodeTrieNodeEdgeList(ImmutableTrieNode* node) {
    size_t num_children = node->children_.size();
    if (num_children < 2 || num_children > MAX_COUNT_EDGE_LIST_) return -1;
    std::shared_ptr<std::vector<uint8_t>> edge_list_encode = std::make_shared<std::vector<uint8_t>>();
    InsertByteList(edge_list_encode);

    int bytes_cnt = 0;

    std::vector<int32_t> offsets;
    offsets.resize(num_children);

    for (int i = 0; i < num_children; i++) {
        if (node->children_[i]->has_data_) {
            bytes_cnt += EncodeData(node->children_[i]->data_);
        }
        offsets[i] = bytes_cnt;
        bytes_cnt += EncodeTrieNode(node->children_[i]);
    }

    std::shared_ptr<std::vector<uint8_t>> list_buffer_latest(list_buffer_);
    list_buffer_ = edge_list_encode;

    int offset_size = ceil(log2(bytes_cnt + 1) + 2 + 7) / 8;

    int tag = BYTE_EDGE_LIST_;
    list_buffer_->emplace_back(tag);
    int tag_edgelist = (offset_size - 1) | num_children << 2;
    list_buffer_->emplace_back(tag_edgelist | BYTE_MAGIC_EDGE_LIST_);

    for (int i = 0; i < num_children; i++) {
        EncodeChar(node->children_[i]->conv_char_);
        EncodeOffset(node->children_[i], offsets[i], offset_size);
    }

    bytes_cnt += list_buffer_->size();
    list_buffer_ = list_buffer_latest;

    return bytes_cnt;
}

int ImmutableTrieConstructor::EncodeTrieNodeEdgeTable(ImmutableTrieNode* node) {
    int num_children = node->children_.size();
    if (num_children <= MAX_COUNT_EDGE_LIST_) return -1;

    std::shared_ptr<std::vector<uint8_t>> edge_list_encode = std::make_shared<std::vector<uint8_t>>();
    InsertByteList(edge_list_encode);

    int bytes_cnt = 0;
    std::vector<int32_t> offsets;
    offsets.resize(num_children);
    for (int i = 0; i < num_children; i++) {
        if (node->children_[i]->has_data_) {
            bytes_cnt += EncodeData(node->children_[i]->data_);
        }
        offsets[i] = bytes_cnt;
        bytes_cnt += EncodeTrieNode(node->children_[i]);
    }

    std::shared_ptr<std::vector<uint8_t>> list_buffer_latest(list_buffer_);
    list_buffer_ = edge_list_encode;

    int offset_size = ceil(log2(bytes_cnt + 1) + 2 + 7) / 8;
    uint32_t empty_offset = 0xFFFFFFFF >> (4 - offset_size) * 8;
    if (static_cast<uint32_t>(bytes_cnt) >= empty_offset >> 2) {
        offset_size++;
        empty_offset = 0xFFFFFFFF >> (4 - offset_size) * 8;
    }

    int tag = BYTE_EDGE_TABLE_;
    list_buffer_->emplace_back(static_cast<uint8_t>(tag));
    int tag_edgetable = (offset_size - 1);
    list_buffer_->emplace_back(static_cast<uint8_t>(tag_edgetable | BYTE_MAGIC_EDGE_TABLE_));

    std::vector<int> table_offset_index;
    table_offset_index.resize(max_char_val_);

    for (int& i : table_offset_index) {
        i = -1;
    }
    for (int i = 0; i < num_children; i++) {
        table_offset_index[translation_[static_cast<uint8_t>(node->children_[i]->conv_char_)] - 1] = i;
    }

    for (const int& i : table_offset_index) {
        if (i == -1) {
            EncodeUINT(empty_offset, offset_size);
        } else {
            int node_index = i;
            EncodeOffset(node->children_[node_index], offsets[node_index], offset_size);
        }
    }
    bytes_cnt += list_buffer_->size();
    list_buffer_ = list_buffer_latest;

    return bytes_cnt;
}

void ImmutableTrieConstructor::CleanUp(ImmutableTrieNode* ptr) {
    if (ptr == nullptr) {
        return;
    }
    for (auto& child : ptr->children_) {
        CleanUp(child);
    }
    delete ptr;
}

uint32_t ImmutableTrie::DecodeUInt32(const TrieData& trie_ptr, uint32_t payload_size) {
    uint32_t mask = 0xffffffff >> ((4 - payload_size) * 8);
    uint32_t val = *(reinterpret_cast<uint32_t*>(trie_ptr));
    return val & mask;
}

uint32_t ImmutableTrie::DecodeData(const TrieData& trie_ptr) {
    return DecodeUInt32(trie_ptr - payload_size_, payload_size_);
}

uint32_t ImmutableTrie::DecodeOffset(const TrieData& trie_ptr, uint32_t offset_size, bool& has_data, int& state) {
    uint32_t offset = DecodeUInt32(trie_ptr, offset_size);
    has_data = (offset & 2) != 0;
    state = (offset & 1);
    return offset >> 2;
}

int ImmutableTrie::DecodeEdgeTable(TrieData& trie_ptr, uint8_t ch, bool& has_data) {
    uint8_t tag_edgetable = *trie_ptr++;

    // read offset size
    uint32_t offset_size = (tag_edgetable & 0x3) + 1;

    uint32_t offset_table = offset_size * max_char_val_;

    // read offset from edge table
    int state;
    uint32_t offset = DecodeOffset(trie_ptr + (ch - 1) * offset_size, offset_size, has_data, state);

    // check the no match case
    if (offset == 0xffffffff >> ((4 - offset_size) * 8 + 2)) {
        return NO_MATCH;
    }

    // set offset to the next node
    trie_ptr += offset_table + offset;
    return state;
}

int ImmutableTrie::DecodeEdgeList(TrieData& trie_ptr, uint8_t ch, bool& has_data) {
    uint8_t tag_edgelist = *trie_ptr++;

    // read offset size
    uint32_t offset_size = (tag_edgelist & 0x3) + 1;

    // read number of edges
    uint32_t edges_cnt = (tag_edgelist >> 2) & 0x0f;

    // pre-calculate the offset of edge list
    uint32_t offset_list = (offset_size + 1) * edges_cnt;

    // match against nodes in list
    uint32_t i;
    TrieData offset_edge = trie_ptr;
    for (i = 0; i < edges_cnt; i++) {
        uint8_t char_decoded = *offset_edge++;
        if (char_decoded == ch) {
            // read offset from edge table
            int state;
            uint32_t offset = DecodeOffset(offset_edge, offset_size, has_data, state);

            // set offset to the next node
            trie_ptr += offset_list + offset;
            return state;
        }

        offset_edge += offset_size;
    }

    return NO_MATCH;
}

int ImmutableTrie::DecodeSingleFollow(uint32_t tag, TrieData& trie_ptr, uint8_t ch, bool& has_data) {
    // decode single follow without data
    if (tag <= max_char_val_) {
        if (tag != ch) {
            return NO_MATCH;
        }
        // must be internal node, as leaf node always has data
        has_data = false;
        return MATCH_INTERNAL;
    }

    // decode single follow without table
    if (single_follow_dec_ == nullptr) {
        tag -= max_char_val_ + 1;

        // decode single follow leaf range
        if (tag <= max_char_val_) {
            if (tag != ch) {
                return NO_MATCH;
            }

            has_data = true;
            trie_ptr += payload_size_;
            return MATCH_LEAF;
        }

        // decode single follow internal range
        tag -= max_char_val_ + 1;

        if (tag != ch) {
            return NO_MATCH;
        }
        has_data = true;
        trie_ptr += payload_size_;
        return MATCH_INTERNAL;
    }

    // decode single follow with table
    if (tag < BYTE_SINGLE_FOLLOW_MIN) {
        uint16_t char_decoded = single_follow_dec_[tag - (max_char_val_ + 1)];
        if (static_cast<uint8_t>(char_decoded) != ch) {
            return NO_MATCH;
        }

        has_data = true;
        trie_ptr += payload_size_;
        return char_decoded >> 8;
    }

    // decode entries out of table range
    uint8_t char_decoded = *trie_ptr++;
    if (char_decoded != ch) {
        return NO_MATCH;
    }

    has_data = true;
    trie_ptr += payload_size_;

    // assuming _byteSingleFollowLeafMore ends with 0
    // and _byteSingleFollowInternalMore ends with 1
    return tag & 1;
}

int ImmutableTrie::Decode(TrieData& trie_ptr, uint8_t ch, bool& has_data) {
    uint32_t tag = *trie_ptr++;

    if (tag <= BYTE_SINGLE_FOLLOW_MAX) {
        return DecodeSingleFollow(tag, trie_ptr, ch, has_data);
    }
    if (tag == BYTE_EDGE_LIST) {
        return DecodeEdgeList(trie_ptr, ch, has_data);
    }
    return DecodeEdgeTable(trie_ptr, ch, has_data);
}

void ImmutableTrie::ListChildren(TrieData trie_ptr,
                                 std::vector<std::tuple<TrieData, uint8_t, uint32_t, int, bool>>& children) {
    TrieData curr_ptr = trie_ptr;
    uint32_t tag = *curr_ptr++;

    if (tag <= BYTE_SINGLE_FOLLOW_MAX) {
        return ListChildrenSingleFollow(curr_ptr, tag, children);
    }
    if (tag == BYTE_EDGE_LIST) {
        return ListChildrenEdgeList(curr_ptr, children);
    }
    return ListChildrenEdgeTable(curr_ptr, children);
}

void ImmutableTrie::ListChildrenEdgeTable(TrieData trie_ptr,
                                          std::vector<std::tuple<TrieData, uint8_t, uint32_t, int, bool>>& children) {
    TrieData curr_ptr = trie_ptr;
    children.clear();

    uint8_t tag_edgetable = *curr_ptr++;

    // read offset size
    uint32_t offset_size = (tag_edgetable & 0x3) + 1;

    uint32_t offset_table = offset_size * max_char_val_;

    for (uint32_t ch = 1; ch <= max_char_val_; ch++) {
        // read offset from edge table
        bool has_data;
        int state;
        uint32_t data = -1;

        uint32_t offset = DecodeOffset(curr_ptr + (ch - 1) * offset_size, offset_size, has_data, state);

        // check the no match case
        if (offset == 0xffffffff >> ((4 - offset_size) * 8 + 2)) {
            continue;
        }

        if (has_data) {
            data = DecodeData(curr_ptr + offset_table + offset);
        }

        children.emplace_back(
            std::make_tuple(curr_ptr + offset_table + offset, static_cast<uint8_t>(ch), data, state, has_data));
    }
}

void ImmutableTrie::ListChildrenEdgeList(TrieData trie_ptr,
                                         std::vector<std::tuple<TrieData, uint8_t, uint32_t, int, bool>>& children) {
    TrieData curr_ptr = trie_ptr;
    children.clear();

    uint8_t tag_edgelist = *curr_ptr++;

    // read offset size
    uint32_t offset_size = (tag_edgelist & 0x3) + 1;

    // read number of edges
    uint32_t edges_cnt = (tag_edgelist >> 2) & 0x0f;

    // pre-calculate the offset of edge list
    uint32_t offset_list = (offset_size + 1) * edges_cnt;

    // match against nodes in list
    uint32_t i;
    TrieData offset_edge = curr_ptr;
    for (i = 0; i < edges_cnt; i++) {
        uint8_t char_decoded = *offset_edge++;

        // read offset from edge table
        bool has_data;
        int state;
        uint32_t data = -1;
        uint32_t offset = DecodeOffset(offset_edge, offset_size, has_data, state);

        if (has_data) {
            data = DecodeData(curr_ptr + offset_list + offset);
        }

        children.emplace_back(std::make_tuple(curr_ptr + offset_list + offset, char_decoded, data, state, has_data));

        offset_edge += offset_size;
    }
}

void ImmutableTrie::ListChildrenSingleFollow(
    TrieData trie_ptr, const uint32_t tag, std::vector<std::tuple<TrieData, uint8_t, uint32_t, int, bool>>& children) {
    TrieData curr_ptr = trie_ptr;
    children.clear();

    // decode single follow without data
    if (tag <= max_char_val_) {
        // must be internal node, as leaf node always has data
        children.emplace_back(
            std::make_tuple(curr_ptr, static_cast<uint8_t>(tag), static_cast<uint32_t>(-1), MATCH_INTERNAL, false));

        return;
    }

    uint32_t teg = tag;
    // decode single follow without table
    if (single_follow_dec_ == nullptr) {
        teg -= max_char_val_ + 1;

        // decode single follow leaf range
        if (teg <= max_char_val_) {
            auto data = static_cast<uint32_t>(-1);
            data = DecodeData(curr_ptr + payload_size_);

            children.emplace_back(
                std::make_tuple(curr_ptr + payload_size_, static_cast<uint8_t>(teg), data, MATCH_LEAF, true));
            return;
        }

        // decode single follow internal range
        teg -= max_char_val_ + 1;

        uint32_t data;
        data = DecodeData(curr_ptr + payload_size_);

        children.emplace_back(
            std::make_tuple(curr_ptr + payload_size_, static_cast<uint8_t>(teg), data, MATCH_INTERNAL, true));
        return;
    }

    // decode single follow with table
    if (teg < BYTE_SINGLE_FOLLOW_MIN) {
        uint16_t char_decoded = single_follow_dec_[teg - (max_char_val_ + 1)];
        auto ch = static_cast<uint8_t>(char_decoded);

        uint32_t data;
        data = DecodeData(curr_ptr + payload_size_);

        children.emplace_back(std::make_tuple(curr_ptr + payload_size_, ch, data, char_decoded >> 8, true));
        return;
    }

    // decode entries out of table range
    uint8_t char_decoded = *curr_ptr++;

    auto data = static_cast<uint32_t>(-1);
    data = DecodeData(curr_ptr + payload_size_);

    // assuming _byteSingleFollowLeafMore ends with 0
    // and _byteSingleFollowInternalMore ends with 1
    children.emplace_back(std::make_tuple(curr_ptr + payload_size_, char_decoded, data, teg & 1, true));
}

void ImmutableTrie::CleanUp() {
    delete[] single_follow_dec_;
    delete[] trie_data_;
}

ImmutableTrie::ImmutableTrie(const std::string& path)
    : trie_data_(nullptr),
      single_follow_dec_(nullptr),
      payload_size_(0),
      trie_data_size_(0),
      detranslate_{0},
      max_char_val_(3),
      size_single_follow_dec_(0),
      translate_{0} {
    auto result = Initialize(path);
    if (result.has_error()) {
        PYIS_THROW(result.error().what());
    }
}

ImmutableTrie::ImmutableTrie(const std::vector<std::tuple<std::string, uint32_t>>& data)
    : trie_data_(nullptr),
      single_follow_dec_(nullptr),
      payload_size_(0),
      trie_data_size_(0),
      detranslate_{0},
      max_char_val_(3),
      size_single_follow_dec_(0),
      translate_{0} {
    ImmutableTrieConstructor constructor(data);
    constructor.WriteToTrie(*this);
}

ImmutableTrie::ImmutableTrie(BinaryReader& reader)
    : trie_data_(nullptr),
      single_follow_dec_(nullptr),
      payload_size_(0),
      trie_data_size_(0),
      detranslate_{0},
      max_char_val_(3),
      size_single_follow_dec_(0),
      translate_{0} {
    Initialize(reader);
}

ImmutableTrie::~ImmutableTrie() { CleanUp(); }

Expected<uint32_t> ImmutableTrie::Match(const uint8_t* match_str, const uint8_t* match_str_end, TrieData& data_ptr) {
    if (trie_data_ == nullptr) {
        return Expected<uint32_t>(std::runtime_error("Trie not initialized"));
    }
    // continue decoding or start from the beginning
    TrieData curr_ptr = (data_ptr == nullptr) ? trie_data_ : data_ptr;
    data_ptr = nullptr;

    // decode string
    int state = NO_MATCH;
    bool has_data = false;
    for (/**/; *match_str != 0U && match_str != match_str_end; match_str++) {
        uint8_t ch = *match_str;

        ch = translate_[ch];

        if (ch == BYTE_NO_MATCH) {
            return Expected<uint32_t>(std::runtime_error("Key not found"));
        }
        uint32_t tag = *curr_ptr++;

        if (tag <= BYTE_SINGLE_FOLLOW_MAX) {
            state = DecodeSingleFollow(tag, curr_ptr, ch, has_data);
        } else if (tag == BYTE_EDGE_LIST) {
            state = DecodeEdgeList(curr_ptr, ch, has_data);
        } else {
            state = DecodeEdgeTable(curr_ptr, ch, has_data);
        }

        if ((state == NO_MATCH) || (state == MATCH_LEAF && match_str != match_str_end - 1)) {
            return Expected<uint32_t>(std::runtime_error("Key not found"));
        }
    }

    if (has_data) {
        uint32_t data = DecodeData(curr_ptr);

        if (state != MATCH_LEAF) {
            state = Decode(curr_ptr, BYTE_SEPERATOR, has_data);

            // match further only for internal nodes
            data_ptr = (state != MATCH_INTERNAL) ? nullptr : curr_ptr;
        }

        return Expected<uint32_t>(data);
    }
    if (state != MATCH_LEAF) {
        state = Decode(curr_ptr, BYTE_SEPERATOR, has_data);
        // match further only for internal nodes
        data_ptr = (state != MATCH_INTERNAL) ? nullptr : curr_ptr;
    }
    return Expected<uint32_t>(std::runtime_error("Key not found"));
}

Expected<uint32_t> ImmutableTrie::Match(const std::string& str, TrieData& data_ptr) {
    return Match(reinterpret_cast<const uint8_t*>(str.c_str()),
                 reinterpret_cast<const uint8_t*>(str.c_str()) + str.length(), data_ptr);
}

Expected<uint32_t> ImmutableTrie::Match(const std::string& str) {
    TrieData tmp = nullptr;
    return Match(str, tmp);
}

bool ImmutableTrie::Contains(const std::string& str) { return Match(str).has_value(); }

std::vector<std::tuple<std::string, uint32_t>> ImmutableTrie::Items() {
    std::vector<std::tuple<std::string, uint32_t>> data;
    if (trie_data_ == nullptr) {
        return data;
    }
    std::stack<std::tuple<TrieData, uint8_t, uint32_t, int, bool>> stk;
    std::stack<size_t> branches;
    std::vector<std::tuple<TrieData, uint8_t, uint32_t, int, bool>> children;
    ListChildren(trie_data_, children);
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        stk.emplace(*it);
    }

    branches.emplace(children.size());

    std::string curr_string;
    while (!stk.empty()) {
        auto curr_node = stk.top();
        stk.pop();
        --branches.top();
        curr_string += static_cast<char>(detranslate_[std::get<1>(curr_node)]);
        if (std::get<4>(curr_node)) {
            data.emplace_back(std::make_tuple(curr_string, std::get<2>(curr_node)));
        }
        if (std::get<3>(curr_node) == MATCH_INTERNAL) {
            ListChildren(std::get<0>(curr_node), children);
            for (auto it = children.rbegin(); it != children.rend(); ++it) {
                stk.emplace(*it);
            }
            branches.emplace(children.size());
        } else if (std::get<3>(curr_node) == MATCH_LEAF) {
            curr_string.pop_back();
        }
        while (!branches.empty() && branches.top() == 0) {
            branches.pop();
            if (!curr_string.empty()) {
                curr_string.pop_back();
            }
        }
    }
    return data;
}

void ImmutableTrie::LoadItems(const std::vector<std::tuple<std::string, uint32_t>>& data) {
    CleanUp();
    single_follow_dec_ = nullptr;
    ImmutableTrieConstructor constructor(data);
    constructor.WriteToTrie(*this);
}

Expected<void> ImmutableTrie::Load(const std::string& path) { return Initialize(path); }

Expected<void> ImmutableTrie::Initialize(const std::string& path) {
    BinaryReader reader(path);
    return Initialize(reader);
}

Expected<void> ImmutableTrie::Initialize(BinaryReader& reader) {
    CleanUp();
    single_follow_dec_ = nullptr;
    reader.ReadUInt32('MCHV', &max_char_val_);
    reader.ReadUInt32('CBPL', &payload_size_);
    reader.ReadBuffer('TLTB', translate_, 256);
    if ((max_char_val_ + 1) * 3 - 1 > BYTE_SINGLE_FOLLOW_MAX) {
        reader.ReadUInt32('TLSL', &size_single_follow_dec_);
        single_follow_dec_ = new uint16_t[size_single_follow_dec_];

        for (uint32_t i = 0; i < size_single_follow_dec_; i++) {
            reader.ReadInt16('TLSF', reinterpret_cast<int16_t*>(single_follow_dec_ + i));
        }
    }
    reader.ReadUInt32('TREL', &trie_data_size_);
    trie_data_ = new uint8_t[trie_data_size_ + 4];
    reader.ReadBuffer('TREN', trie_data_, trie_data_size_);
    for (uint8_t& i : detranslate_) {
        i = ' ';
    }
    for (size_t i = 0; i < TRANSLATE_TABLE_SIZE; i++) {
        detranslate_[translate_[i]] = i;
    }

    detranslate_[translate_[' ']] = ' ';
    detranslate_[BYTE_NO_MATCH] = ' ';
    return Expected<void>();
}

Expected<void> ImmutableTrie::Compile(const std::vector<std::tuple<std::string, uint32_t>>& data,
                                      const std::string& path) {
    ImmutableTrieConstructor constructor(data);
    return constructor.WriteToFile(path);
}

void ImmutableTrie::Save(const std::string& path) {
    BinaryWriter writer(path);
    Save(writer);
}

void ImmutableTrie::Save(BinaryWriter& writer) {
    if (trie_data_ == nullptr) {
        // Not initialized. Do nothing.
        return;
    }
    writer.WriteUInt32('MCHV', max_char_val_);
    writer.WriteUInt32('CBPL', payload_size_);
    writer.WriteBuffer('TLTB', translate_, 256);
    if (single_follow_dec_ != nullptr) {
        writer.WriteUInt32('TSTL', size_single_follow_dec_);
        for (int i = 0; i < size_single_follow_dec_; i++) {
            writer.WriteInt16('TLSF', static_cast<int16_t>(single_follow_dec_[i]));
        }
    }

    writer.WriteUInt32('TREL', trie_data_size_);
    writer.WriteBuffer('TREN', trie_data_, trie_data_size_);
}

std::string ImmutableTrie::Serialize(ModelStorage& storage) {
    std::string data_file = storage.uniq_file("immutable_trie", ".data.bin");
    BinaryWriter writer(storage.open_ostream(data_file));

    Save(writer);

    JsonPersistHelper jph(1);
    jph.add_file("data", data_file);

    std::string config_file = storage.uniq_file("immutable_trie", ".config.json");
    std::string state = jph.serialize(config_file, storage);
    return state;
}

void ImmutableTrie::Deserialize(const std::string& state, ModelStorage& storage) {
    JsonPersistHelper jph(state, storage);
    int version = jph.version();
    if (1 == version) {
        std::string data_file = jph.get_file("data");
        BinaryReader reader(storage.open_istream(data_file));
        Initialize(reader);
        return;
    }

    PYIS_THROW("ImmutableTrie v%d is incompatible with the runtime", version);
}

}  // namespace ops
}  // namespace pyis