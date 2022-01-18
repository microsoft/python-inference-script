#include <climits>
#include <cstring>
#include <ctime>
#include <fstream>
#include <string>
#include <unordered_map>

#include "gtest/gtest.h"
#include "pyis/ops/text/cedar_trie.h"
#include "pyis/ops/tokenizer/gpt2_tokenizer.h"
TEST(TestCedarTrie, Basic) {
    pyis::ops::CedarTrie trie;
    trie.Insert("Alpha", 1);
    trie.Insert("Beta", 2);
    trie.Insert("Delta", 3);
    trie.Insert("AlphaBeta", 4);

    ASSERT_EQ(trie.Predict("Alpha").size(), 2);
    ASSERT_EQ(trie.Predict("Gamma").size(), 0);
    ASSERT_EQ(trie.NumKeys(), 4);
    ASSERT_EQ(trie.Lookup("Alpha").value(), 1);
    ASSERT_EQ(trie.Lookup("Beta").value(), 2);
    ASSERT_EQ(trie.Lookup("Delta").value(), 3);
    ASSERT_EQ(trie.Lookup("AlphaBeta").value(), 4);
    ASSERT_TRUE(trie.Lookup("Gamma").has_error());

    trie.Insert("Gamma", 5);
    trie.Insert("Lambda", 6);
    trie.Insert("Alpha", 7);

    ASSERT_EQ(trie.NumKeys(), 6);
    ASSERT_EQ(trie.Lookup("Alpha").value(), 7);

    trie.Erase("Gamma");

    ASSERT_TRUE(trie.Lookup("Gamma").has_error());
    ASSERT_EQ(trie.Lookup("Lambda").value(), 6);

    trie.Insert("Sentence with Space and\tTabs", 8);
    ASSERT_EQ(trie.Lookup("Sentence with Space and\tTabs").value(), 8);
    ASSERT_EQ(trie.Predict("Sentence ").size(), 1);

    trie.Insert("   ", 9);
    trie.Insert(" \t", 10);
    trie.Insert("\t ", 11);
    ASSERT_EQ(trie.Predict(" ").size(), 2);
    ASSERT_EQ(trie.Lookup("\t ").value(), 11);
}

TEST(TestCedarTrie, Dump) {
    std::ifstream fin("tests/test_cedar_trie/data/wordlist.txt");
    int cnt = 0;
    std::string token;
    pyis::ops::CedarTrie trie;
    std::unordered_map<std::string, int> dir;
    while (fin >> token) {
        ++cnt;
        trie.Insert(token, cnt);
        dir[token] = cnt;
    }
    fin.close();
    cnt = dir.size();
    ASSERT_EQ(trie.NumKeys(), cnt);

    auto table = trie.Items();
    ASSERT_EQ(table.size(), cnt);

    for (auto& ite : table) {
        ASSERT_EQ(dir.count(std::get<0>(ite)), 1);
        ASSERT_EQ(dir[std::get<0>(ite)], std::get<1>(ite));
    }
}

TEST(TestCedarTrie, RestoreFromFile) {
    std::ifstream fin("tests/test_cedar_trie/data/wordlist.txt");
    int cnt = 0;
    std::string token;
    pyis::ops::CedarTrie trie;
    std::unordered_map<std::string, int> dir;
    while (fin >> token) {
        ++cnt;
        trie.Insert(token, cnt);
        dir[token] = cnt;
    }
    fin.close();

    system("mkdir tmp");
    std::ofstream fout("tmp/trie.bin", std::ios::binary);
    trie.Save(fout);
    fout.close();

    Cedar::Trie trie2;
    fin = std::ifstream("tmp/trie.bin", std::ios::binary);
    ASSERT_EQ(fin.good(), true);
    trie2.Open(fin);
    fin.close();

    std::vector<std::tuple<std::string, int>> res1;
    std::vector<std::tuple<std::string, int>> res2;
    res1 = trie.Items();
    res2 = trie.Items();
    ASSERT_EQ(res1, res2);
}

TEST(TestCedarTrie, PrefixPredict) {
    std::ifstream fin("tests/test_cedar_trie/data/wordlist.txt");
    int cnt = 0;
    std::string token;
    pyis::ops::CedarTrie trie;
    std::unordered_map<std::string, int> dir;
    std::vector<std::tuple<std::string, int>> wordlist;
    while (fin >> token) {
        ++cnt;
        trie.Insert(token, cnt);
        dir[token] = cnt;
        wordlist.emplace_back(std::make_tuple(token, cnt));
    }
    fin.close();

    // Predict
    std::vector<std::tuple<std::string, int>> res;
    res = trie.Predict("m");

    int anscnt = 0;
    for (auto& ite : dir) {
        if (ite.first.length() > 0 && ite.first[0] == 'm') anscnt++;
    }
    ASSERT_EQ(anscnt, res.size());
    for (auto& ite : res) {
        ASSERT_EQ(dir.count(std::get<0>(ite)), 1);
        ASSERT_EQ(dir[std::get<0>(ite)], std::get<1>(ite));
    }

    // Prefix
    res.clear();

    auto rand32 = []() -> unsigned { return (rand() & 0xffff << 16) | rand() & 0xffff; };
    auto starts_with = [](const std::string& prefix, const std::string& text) -> bool {
        return text.substr(0, prefix.length()) == prefix;
    };
    for (int i = 0; i < 10000; i++) {
        std::string query;
        for (int j = 0; j < 10; j++) {
            query += std::get<0>(wordlist[(rand32() % wordlist.size())]);
        }
        auto result = trie.Prefix(query);
        for (auto& x : result) {
            ASSERT_EQ(starts_with(std::get<0>(x), query), true);
            ASSERT_EQ(dir.count(std::get<0>(x)), 1);
            ASSERT_EQ(dir[std::get<0>(x)], std::get<1>(x));
        }
        // LongestPrefix;
        auto query_result = trie.LongestPrefix(query);
        ASSERT_TRUE(query_result.has_value());
        std::string longest;
        for (auto& x : result)
            if (std::get<0>(x).length() > longest.length()) longest = std::get<0>(x);
        ASSERT_EQ(longest, std::get<0>(query_result.value()));
        ASSERT_EQ(dir[longest], std::get<1>(query_result.value()));
    }
}

TEST(TestGPT2Tokenizer, Basic) {
    pyis::ops::GPT2Tokenizer tokenizer("D:\\vocab.json", "D:\\merges.txt");
    tokenizer.Tokenize("hello world");
}