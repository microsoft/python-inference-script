// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "Common.h"
#include "Word.h"

namespace SparseLinearChainCRF {
// ------------------------------------------------
// Sentence class.A container of sequence data.
// ------------------------------------------------
template <typename T>
class Sentence {
  public:
    Sentence() = default;

    void Append(Word<T>& word) { m_Words.push_back(std::move(word)); }

    void Clear() {
        m_Words.clear();
        m_ActiveTagset.clear();
    }

    Word<T>& GetWord(const int idx) { return m_Words[idx]; }

    const Word<T>& GetWord(const int idx) const { return m_Words[idx]; }

    size_t Size() const { return m_Words.size(); }

    void Resize(size_t n, const Word<T>& value) { m_Words.resize(n, value); }

    void AddActiveTag(uint16_t label) { m_ActiveTagset.push_back(label); }

    const std::vector<uint16_t>& GetActiveTagset() const { return m_ActiveTagset; }

  private:
    std::vector<Word<T>> m_Words;
    std::vector<uint16_t> m_ActiveTagset;
};

typedef std::tuple<uint8_t, uint32_t, float> MLGFeatureType;
typedef std::pair<size_t, float> IndexedParameterType;

typedef Sentence<MLGFeatureType> MLGFeatureSentence;
typedef Sentence<IndexedParameterType> IndexedSentence;
}  // namespace SparseLinearChainCRF
