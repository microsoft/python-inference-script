// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once
#include <algorithm>
#include <cmath>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <tuple>
#include <vector>

#include "pyis/share/cached_object.h"
#include "pyis/share/expected.hpp"
#include "pyis/share/json_persist_helper.h"
#include "pyis/share/model_storage_local.h"

namespace pyis {

namespace ops {

class BinaryReader {
  public:
    Expected<void> ReadTaggedValue(uint32_t, uint32_t*);
    Expected<void> ReadInt16(uint32_t, int16_t*);
    Expected<void> ReadUInt32(uint32_t, uint32_t*);
    Expected<void> ReadInt32(uint32_t, int32_t*);
    Expected<void> ReadBuffer(uint32_t, uint8_t*, uint32_t);

    explicit BinaryReader(const std::string&);
    explicit BinaryReader(std::shared_ptr<std::istream>);
    ~BinaryReader();

  private:
    int read_cnt_;
    int capacity_;
    bool lower_nibble_;
    uint8_t buffer_[256];
    std::shared_ptr<std::istream> ifs_;
    std::filebuf* file_buffer_;
    size_t ReadBuffer();
    Expected<void> DecodeUInt32(uint32_t*);
};

class BinaryWriter {
  public:
    explicit BinaryWriter(const std::string&);
    explicit BinaryWriter(std::shared_ptr<std::ostream>);
    ~BinaryWriter();

    void SetNibble(uint8_t*, int, uint8_t);
    void WriteBuffer(const int&);
    void WriteNibble(uint8_t);

    void EncodeUINT(uint32_t);

    void WriteTaggedValue(uint32_t, uint32_t);
    void WriteBool(uint32_t, bool);
    void WriteInt16(uint32_t, int16_t);
    void WriteInt32(uint32_t, int32_t);
    void WriteUInt32(uint32_t, uint32_t);
    void WriteBuffer(uint32_t, const uint8_t*, uint32_t);

  private:
    int used_;
    std::shared_ptr<std::ostream> ofs_;
    std::filebuf* file_buffer_;

    static const int BUFFER_LEN = 256;
    uint8_t buffer_[BUFFER_LEN];
};

struct ImmutableTrieNode {
    bool has_data_;
    uint32_t data_;
    std::vector<ImmutableTrieNode*> children_;
    char conv_char_;

    void AddChild(const std::string& str, int idx, uint32_t data);
    void SetData(uint32_t data);
    ImmutableTrieNode();
    explicit ImmutableTrieNode(char conv_char);
};

class ImmutableTrie;

class ImmutableTrieConstructor {
  public:
    explicit ImmutableTrieConstructor(const std::vector<std::tuple<std::string, uint32_t>>& data);
    ~ImmutableTrieConstructor();
    Expected<void> WriteToFile(const std::string& path);
    Expected<void> WriteToFile(std::shared_ptr<std::ostream>& os);
    Expected<void> WriteToFile(BinaryWriter& bw);
    void WriteToTrie(ImmutableTrie& trie);

  private:
    ImmutableTrieNode* root_;

    uint8_t translation_[256];
    uint8_t translator_[256];

    uint32_t hist_char_counts_[256];
    uint32_t hist_char_counts_single_follow_data_leaf_[256];
    uint32_t hist_char_counts_single_follow_data_internal_[256];

    int max_char_val_;
    int payload_size_;

    std::shared_ptr<std::vector<uint8_t>> list_buffer_;
    std::vector<std::shared_ptr<std::vector<uint8_t>>> list_trie_data_;
    uint16_t* single_follow_enc_ = nullptr;
    std::vector<uint8_t> single_follow_dec_;
    std::vector<uint8_t> trie_data_;

    const uint8_t BYTE_EDGE_TABLE_ = 0xff;
    const uint8_t BYTE_EDGE_LIST_ = 0xfe;
    const uint8_t BYTE_SINGLE_FOLLOW_MAX_ = 0xfd;
    const uint8_t BYTE_SINGLE_FOLLOW_INTERNAL_MORE_ = 0xfd;
    const uint8_t BYTE_SINGLE_FOLLOW_LEAF_MORE_ = 0xfc;
    const uint8_t BYTE_SINGLE_FOLLOW_MIN_ = 0xfc;

    const uint8_t BYTE_NO_MATCH_ = 0;
    const uint8_t BYTE_SEPERATOR_ = 1;
    const uint8_t BYTE_ANY_WORD_ = 2;
    const uint8_t BYTE_ANY_CHAR_ = 3;

    const uint8_t TRIE_SEPERATOR_ = ' ';
    const uint8_t TRIE_ANY_WORD_ = '*';
    const uint8_t TRIE_ANY_CHAR_ = '?';

    const size_t MAX_COUNT_EDGE_LIST_ = 15;

    const uint8_t BYTE_MAGIC_EDGE_LIST_ = 0x80;   // 1000 0000
    const uint8_t BYTE_MAGIC_EDGE_TABLE_ = 0xAC;  // 1010 1100

    void BuildHistChildrenCounts(ImmutableTrieNode* node);
    void BuildTranslator();
    void BuildSingleFollowTranslator();

    int EncodeTrieNode(ImmutableTrieNode* node);
    void EncodeChar(const uint8_t& ch);
    int EncodeData(const uint32_t& data);
    int EncodeOffset(ImmutableTrieNode* node, const int& offset, const int&);
    int EncodeUINT(uint32_t x, int);
    size_t EncodeTrieNodeSingleFollow(ImmutableTrieNode* node);
    void InsertByteList(const std::shared_ptr<std::vector<uint8_t>>& content);
    int EncodeTrieNodeEdgeList(ImmutableTrieNode* node);
    int EncodeTrieNodeEdgeTable(ImmutableTrieNode* node);

    void CleanUp(ImmutableTrieNode* ptr);
};

class ImmutableTrie : public CachedObject<ImmutableTrie> {
  public:
    using TrieData = uint8_t*;
    friend class ImmutableTrieConstructor;

    ImmutableTrie() = default;  // For deserialize only
    explicit ImmutableTrie(const std::string& path);
    explicit ImmutableTrie(const std::vector<std::tuple<std::string, uint32_t>>& data);
    explicit ImmutableTrie(BinaryReader& reader);
    ~ImmutableTrie();
    Expected<uint32_t> Match(const uint8_t*, const uint8_t*, TrieData&);
    Expected<uint32_t> Match(const std::string& str, TrieData&);
    Expected<uint32_t> Match(const std::string& str);
    bool Contains(const std::string& str);

    std::vector<std::tuple<std::string, uint32_t>> Items();
    void LoadItems(const std::vector<std::tuple<std::string, uint32_t>>& data);
    Expected<void> Load(const std::string& path);

    static Expected<void> Compile(const std::vector<std::tuple<std::string, uint32_t>>& data, const std::string& path);
    void Save(const std::string& path);
    void Save(BinaryWriter& writer);
    void Deserialize(const std::string& state, ModelStorage& storage);
    std::string Serialize(ModelStorage& storage);

  private:
    static const uint8_t BYTE_NO_MATCH = 0;
    static const uint8_t BYTE_SEPERATOR = 1;
    static const uint8_t BYTE_ANY_WORD = 2;
    static const uint8_t BYTE_ANY_CHAR = 3;

    // reserved values for trie node type
    static const uint8_t BYTE_EDGE_TABLE = 0xff;
    static const uint8_t BYTE_EDGE_LIST = 0xfe;
    static const uint8_t BYTE_SINGLE_FOLLOW_MAX = 0xfd;
    static const uint8_t BYTE_SINGLE_FOLLOW_INTERNAL_MORE = 0xfd;  // last bit must be 1
    static const uint8_t BYTE_SINGLE_FOLLOW_LEAF_MORE = 0xfc;      // last bit must be 0
    static const uint8_t BYTE_SINGLE_FOLLOW_MIN = 0xfc;

    // magic fields for verification
    static const uint8_t BYTE_MAGIC_EDGE_LIST = 0x80;   // 1000 0000
    static const uint8_t BYTE_MAGIC_EDGE_TABLE = 0xAC;  // 1010 1100

    // trie matching state
    static const int32_t NO_MATCH = -1;
    static const int32_t MATCH_LEAF = 0;      // must be 0 due to optimization
    static const int32_t MATCH_INTERNAL = 1;  // must be 1 due to optimization

    // put frequently used members together to optimize cache performance

    // max value for char encoding
    uint32_t max_char_val_;
    // size of payload
    uint32_t payload_size_;
    // LUT for single follow decoding
    uint16_t* single_follow_dec_;
    // byte array encoding of trie
    uint8_t* trie_data_;
    // size of trie data byte array
    uint32_t trie_data_size_;
    // size of single follow decoding LUT
    uint32_t size_single_follow_dec_;

    // translation table
    static constexpr int TRANSLATE_TABLE_SIZE = 256;
    uint8_t translate_[TRANSLATE_TABLE_SIZE];
    uint8_t detranslate_[TRANSLATE_TABLE_SIZE];

    uint32_t DecodeUInt32(const TrieData&, uint32_t);
    uint32_t DecodeData(const TrieData&);
    uint32_t DecodeOffset(const TrieData&, uint32_t, bool&, int& state);

    int DecodeEdgeTable(TrieData&, uint8_t ch, bool&);
    int DecodeEdgeList(TrieData&, uint8_t ch, bool&);
    int DecodeSingleFollow(uint32_t tag, TrieData&, uint8_t ch, bool&);
    int Decode(TrieData&, uint8_t ch, bool&);

    void ListChildren(TrieData, std::vector<std::tuple<TrieData, uint8_t, uint32_t, int, bool>>& children);
    void ListChildrenEdgeTable(TrieData, std::vector<std::tuple<TrieData, uint8_t, uint32_t, int, bool>>& children);
    void ListChildrenEdgeList(TrieData, std::vector<std::tuple<TrieData, uint8_t, uint32_t, int, bool>>& children);
    void ListChildrenSingleFollow(TrieData, uint32_t tag,
                                  std::vector<std::tuple<TrieData, uint8_t, uint32_t, int, bool>>& children);
    void CleanUp();

    Expected<void> Initialize(BinaryReader& reader);
    Expected<void> Initialize(const std::string& path);
};

}  // namespace ops
}  // namespace pyis