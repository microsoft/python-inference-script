#include <codecvt>
#include <fstream>
#include <locale>

#include "gtest/gtest.h"
#include "pyis/share/json_persist_helper.h"
#include "pyis/share/model_storage_local.h"

TEST(TestJsonPersistHelper, SignVersion) {
    pyis::JsonPersistHelper jph(1);
    std::string signature = jph.sign();
    ASSERT_EQ(signature, "dc179a0cfa65b15b52a4ee6021fcb831");
}

TEST(TestJsonPersistHelper, SignBool) {
    pyis::JsonPersistHelper jph(2);
    jph.add<bool>("die", true);
    std::string signature = jph.sign();
    ASSERT_EQ(signature, "34972481a143d3012a2065a78786e62c");
}

TEST(TestJsonPersistHelper, SignInt) {
    pyis::JsonPersistHelper jph(1);
    jph.add("answer", 42);
    std::string signature = jph.sign();
    ASSERT_EQ(signature, "01dffcf45c82a5c564fbdfa864ca7f3e");
}

TEST(TestJsonPersistHelper, SignUInt) {
    pyis::JsonPersistHelper jph(1);
    jph.add<unsigned int>("max_uint", 0xffffffff);
    std::string signature = jph.sign();
    ASSERT_EQ(signature, "1080d443024ac0b3a83449d4abce23ef");
}

TEST(TestJsonPersistHelper, SignInt64) {
    pyis::JsonPersistHelper jph(1);
    jph.add<int64_t>("min_int64", int64_t(-9223372036854775808LL));
    std::string signature = jph.sign();
    ASSERT_EQ(signature, "98cde62af03eac79cc5266a26505a483");
}

TEST(TestJsonPersistHelper, SignUInt64) {
    pyis::JsonPersistHelper jph(1);
    jph.add<uint64_t>("max_uint64", 0xFFFFFFFFFFFFFFFF);
    std::string signature = jph.sign();
    ASSERT_EQ(signature, "c66fb4661f5ef3ce88bc40ceb030c7ef");
}

TEST(TestJsonPersistHelper, SignString) {
    pyis::JsonPersistHelper jph(1);
    jph.add("key", "value");
    std::string signature = jph.sign();
    ASSERT_EQ(signature, "6c3590e080b46659ac78b8fb1a844e65");
}

TEST(TestJsonPersistHelper, SignList) {
    pyis::JsonPersistHelper jph(1);
    std::vector<int> l = {1, 2, 3};
    jph.add("list", l);
    std::string signature = jph.sign();
    ASSERT_EQ(signature, "8314db700c5165cfd22603d3444e3786");
}

TEST(TestJsonPersistHelper, SignSorted) {
    pyis::JsonPersistHelper jph1(1);
    jph1.add("key1", "value1");
    jph1.add("key2", "value2");

    pyis::JsonPersistHelper jph2(1);
    jph2.add("key2", "value2");
    jph2.add("key1", "value1");

    ASSERT_EQ(jph1.sign(), jph2.sign());
}

TEST(TestJsonPersistHelper, SignFile) {
    pyis::JsonPersistHelper jph(1);
    pyis::ModelStorageLocal storage("tests/test_share/data");
    jph.add("config:file", "file_without_newline.for_cache_test.txt");
    std::string signature = jph.sign(&storage);
    ASSERT_EQ(signature, "e1db69a040ec6dbac5da37ab00506503");
}
