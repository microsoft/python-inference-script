// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "Common.h"
#include "DataFormatter.h"
#include "Sentence.h"
#include "SparseLinearModel.h"

namespace SparseLinearChainCRF {
// ------------------------------------------------
// StreamDataManager
// ------------------------------------------------
class StreamDataManager {
  public:
    StreamDataManager(const std::string filename, std::shared_ptr<SparseLinearModel> model,
                      const std::string workingFolder = "tmp", int chunkSize = 100000, bool expandParameter = false,
                      bool shuffleData = false);
    void Flush();
    const std::vector<IndexedSentence>& Next();
    bool Empty() { return m_ProcessingQueue.empty(); }
    size_t Size() { return m_TotalDataSize; }
    size_t ChunkSize() { return m_ChunkFilenames.size(); }

  private:
    void Save(const std::string filename, const std::vector<IndexedSentence>& indexedData) const;
    void Load(const std::string filename, std::vector<IndexedSentence>& indexedData) const;

    std::vector<std::string> m_ChunkFilenames;
    std::queue<size_t> m_ProcessingQueue;
    int m_ChunkSize;
    size_t m_TotalDataSize;
    bool m_IsReady;
    bool m_ShuffleData;
    std::string m_WorkingFolder;

    std::vector<std::vector<IndexedSentence>> m_InMemoryChunk;
    int cur, next;
};
}  // namespace SparseLinearChainCRF