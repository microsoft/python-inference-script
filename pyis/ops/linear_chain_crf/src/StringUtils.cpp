// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "StringUtils.h"

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "Common.h"

namespace SparseLinearChainCRF {
using namespace std;

vector<string> StringUtils::Split(const string& str, const string& delim, bool removeEmptyEntries) {
    vector<string> out;
    out.reserve(5);
    string::size_type lpos = 0;
    string::size_type pos = str.find_first_of(delim, lpos);

    while (lpos != string::npos) {
        if (pos != lpos || !removeEmptyEntries) {
            if (pos == string::npos) {
                out.emplace_back(str.substr(lpos));
            } else {
                out.emplace_back(str.substr(lpos, pos - lpos));
            }
        }

        lpos = (pos == string::npos) ? string::npos : pos + 1;
        pos = str.find_first_of(delim, lpos);
    }

    return out;
}

// Naive implementation for now
bool StringUtils::ToBool(const string& str) { return str == "True" || str == "true" || str == "1"; }
}  // namespace SparseLinearChainCRF