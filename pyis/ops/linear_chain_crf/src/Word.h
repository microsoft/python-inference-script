// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <string>
#include <vector>

#include "Common.h"

namespace SparseLinearChainCRF {
// ------------------------------------------------
// Word class.
// "Word" represents a single unit in sequence data, and
// contains a label, importance (instance weight), tag (debug),
// and bag of features.
// ------------------------------------------------
template <typename T>
class Word {
  public:
    Word(uint16_t label, float importance, std::string text = "")
        : m_Label(label), m_Importance(importance), m_Text(text) {}

    void Append(T item) { m_Observation.push_back(std::move(item)); }

    const T& GetFeature(const int idx) const { return m_Observation[idx]; }

    float GetImportance() const { return m_Importance; }

    uint16_t GetLabel() const { return m_Label; }

    std::string GetText() const { return m_Text; }

    void Resize(int size) { m_Observation.resize(size); }

    size_t Size() const { return m_Observation.size(); }

    T& operator[](const int idx) { return m_Observation[idx]; }

    const std::vector<T>& Features() const { return m_Observation; }

  private:
    uint16_t m_Label;

    std::string m_Text;

    float m_Importance;

    std::vector<T> m_Observation;
};
}  // namespace SparseLinearChainCRF