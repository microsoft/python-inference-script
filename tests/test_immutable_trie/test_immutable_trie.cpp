#include <fstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "gtest/gtest.h"
#include "pyis/ops/text/immutable_trie.h"

TEST(ImmutableTrie, Basic) {
    // Generate dict
    uint32_t cnt = 0;
    std::string str;
    const char charlist[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()";
    std::vector<std::tuple<std::string, uint32_t>> data;
    std::unordered_map<std::string, uint32_t> dict;
    for (int i = 0; i < 15000; i++) {
        int len = rand() % 15 + 5;
        str = "";
        for (int j = 0; j < len; j++) {
            str += charlist[rand() % 72];
        }
        data.emplace_back(std::make_tuple(str, cnt));
        dict[str] = cnt;
        cnt++;
    }
    system("mkdir tmp");
    pyis::ops::ImmutableTrie::Compile(data, "tmp/trie.bin");

    pyis::ops::ImmutableTrie trie("tmp/trie.bin");

    auto dumped = trie.Items();
    for (const auto& x : dumped) {
        ASSERT_TRUE(trie.Match(std::get<0>(x)));
    }
    ASSERT_EQ(dumped.size(), dict.size());
    for (const auto& x : dumped) {
        ASSERT_EQ(dict.count(std::get<0>(x)), 1);
        ASSERT_EQ(dict[std::get<0>(x)], std::get<1>(x));
        auto match_result = trie.Match(std::get<0>(x));
        ASSERT_FALSE(match_result.has_error());
        ASSERT_EQ(std::get<1>(x), match_result.value());
    }
}
