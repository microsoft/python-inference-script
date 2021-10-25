// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <cstdio>
#include "pyis/ops/text/cedar_trie.h"

int main() {
    pyis::ops::CedarTrie trie;
    trie.Insert("the answer", 42);
    auto answer = trie.Lookup("the answer");
    printf("the answer is %d", answer.value());
    return 0;
}