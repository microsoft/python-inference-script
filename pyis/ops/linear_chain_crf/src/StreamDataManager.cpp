// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "StreamDataManager.h"

#include <fstream>
#include <random>
#include <string>
#include <vector>

#include "Common.h"
#include "StringUtils.h"

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#if defined(_WIN32) || defined(__unix__)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

namespace SparseLinearChainCRF {
using namespace std;

namespace {
#define RANDOMSEED 0
}  // namespace

StreamDataManager::StreamDataManager(const string filename, shared_ptr<SparseLinearModel> model,
                                     const string workingFolder, int chunkSize, bool expandParameter, bool shuffleData)
    : m_WorkingFolder(workingFolder),
      m_ChunkSize(chunkSize),
      m_TotalDataSize(0),
      m_IsReady(true),
      m_ShuffleData(shuffleData),
      cur(0),
      next(1) {
    cout << "# Processing the data (chunk size=" + std::to_string(chunkSize) + "): " << filename << endl;
    cout << "  Data chunk: ";
    cout.flush();
    if (!fs::is_directory(workingFolder)) fs::create_directory(fs::path(workingFolder));

    int chunkId = 0;
    ifstream fin(filename, ios::in);
    while (!fin.eof()) {
        cout << chunkId << " ";
        cout.flush();
        vector<MLGFeatureSentence> mlgData = DataFormatter::Read(fin, m_ChunkSize);
        m_TotalDataSize += mlgData.size();
        if (mlgData.size() == 0) break;

        if (expandParameter) {
            model->Expand(mlgData);
        }
        vector<IndexedSentence> indexedData = DataFormatter::IndexData(model, mlgData);
        mlgData.clear();

        fs::path targetpath(m_WorkingFolder), sourcepath(filename);
        string chunkFilename = "__" + std::to_string(chunkId) + "__" + sourcepath.filename().string();
        targetpath = targetpath /= chunkFilename;
        Save(targetpath.string(), indexedData);
        m_ChunkFilenames.push_back(targetpath.string());
        vector<size_t> order(indexedData.size());
        for (int i = 0; i < order.size(); ++i) order[i] = i;
        chunkId++;
        indexedData.clear();
    }
    fin.close();
    cout << endl;

    m_InMemoryChunk.resize(2);
    Flush();

    // special case when data fits into memory
    if (m_ChunkFilenames.size() <= 2) {
        Load(m_ChunkFilenames[0], m_InMemoryChunk[0]);
        if (m_ChunkFilenames.size() == 2) Load(m_ChunkFilenames[1], m_InMemoryChunk[1]);
    }
}

void StreamDataManager::Flush() {
    while (!m_ProcessingQueue.empty()) m_ProcessingQueue.pop();

    if (m_ShuffleData)
        std::shuffle(m_ChunkFilenames.begin(), m_ChunkFilenames.end(), std::default_random_engine(RANDOMSEED));

    for (int i = 0; i < ChunkSize(); ++i) m_ProcessingQueue.push(i);

    if (ChunkSize() == 1) {
        cur = next = 0;
    } else if (ChunkSize() == 2) {
        vector<int> chunkIndex = {0, 1};
        if (m_ShuffleData) std::shuffle(chunkIndex.begin(), chunkIndex.end(), std::default_random_engine(RANDOMSEED));
        next = chunkIndex[0];
        cur = chunkIndex[1];
    } else {
        // must refresh in-memory data when data doesn't fit (i.e. chunk size > 2)
        next = 0;
        cur = 1;
        Load(m_ChunkFilenames[m_ProcessingQueue.front()], m_InMemoryChunk[next]);
        m_IsReady = true;
    }
}

const vector<IndexedSentence>& StreamDataManager::Next() {
    m_ProcessingQueue.pop();  // pop cur

    while (!m_IsReady)
        ;
    swap(next, cur);
    if (ChunkSize() > 2 && !m_ProcessingQueue.empty()) {
        m_IsReady = false;
        Load(m_ChunkFilenames[m_ProcessingQueue.front()], m_InMemoryChunk[next]);  // todo : threading
        m_IsReady = true;
    }

    if (m_ShuffleData) {
        std::shuffle(m_InMemoryChunk[cur].begin(), m_InMemoryChunk[cur].end(), std::default_random_engine(RANDOMSEED));
    }

    return m_InMemoryChunk[cur];
}

void StreamDataManager::Save(const string filename, const vector<IndexedSentence>& indexedData) const {
    ofstream stream(filename, ios::out | ios::binary);
    size_t indexedDataSize = indexedData.size();
    stream.write((char*)&indexedDataSize, sizeof(size_t));
    // Sentence
    for (const auto& sentence : indexedData) {
        // ActiveTag
        uint16_t activeTagsetSize = (uint16_t)sentence.GetActiveTagset().size();
        stream.write((char*)&activeTagsetSize, sizeof(uint16_t));
        for (const auto& tag : sentence.GetActiveTagset()) {
            stream.write((char*)&tag, sizeof(uint16_t));
        }
        // Word
        size_t sentenceSize = sentence.Size();
        stream.write((char*)&sentenceSize, sizeof(size_t));
        for (int i = 0; i < sentenceSize; ++i) {
            const auto& word = sentence.GetWord(i);
            uint16_t label = word.GetLabel();
            stream.write((char*)&label, sizeof(uint16_t));
            float importance = word.GetImportance();
            stream.write((char*)&importance, sizeof(float));
            size_t featureSize = word.Features().size();
            stream.write((char*)&featureSize, sizeof(size_t));
            for (const auto& feature : word.Features()) {
                size_t fid = feature.first;
                stream.write((char*)&fid, sizeof(size_t));
                float fval = feature.second;
                stream.write((char*)&fval, sizeof(float));
            }
        }
    }
    stream.close();
}

void StreamDataManager::Load(const string filename, vector<IndexedSentence>& indexedData) const {
    indexedData.clear();

    ifstream stream(filename, ios::in | ios::binary);
    size_t indexedDataSize = 0;
    stream.read((char*)&indexedDataSize, sizeof(size_t));
    // Sentence
    for (size_t sent = 0; sent < indexedDataSize; ++sent) {
        IndexedSentence sentence;
        // ActiveTag
        uint16_t activeTagsetSize = 0;
        stream.read((char*)&activeTagsetSize, sizeof(uint16_t));
        for (int t = 0; t < activeTagsetSize; ++t) {
            uint16_t tag = 0;
            stream.read((char*)&tag, sizeof(uint16_t));
            sentence.AddActiveTag(tag);
        }
        // Word
        size_t sentenceSize = 0;
        stream.read((char*)&sentenceSize, sizeof(size_t));
        for (int i = 0; i < sentenceSize; ++i) {
            uint16_t label = 0;
            stream.read((char*)&label, sizeof(uint16_t));
            float importance = 0.0f;
            stream.read((char*)&importance, sizeof(float));
            Word<IndexedParameterType> word(label, importance, "");

            size_t featureSize = 0;
            stream.read((char*)&featureSize, sizeof(size_t));
            for (size_t j = 0; j < featureSize; ++j) {
                size_t fid = 0;
                stream.read((char*)&fid, sizeof(size_t));
                float fval = 0.0f;
                stream.read((char*)&fval, sizeof(float));
                word.Append(make_pair(fid, fval));
            }
            sentence.Append(word);
        }
        indexedData.push_back(sentence);
    }
    stream.close();
}

}  // namespace SparseLinearChainCRF