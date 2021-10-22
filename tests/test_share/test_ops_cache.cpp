#include <codecvt>
#include <fstream>
#include <locale>

#include "gtest/gtest.h"
#include "pyis/ops/example_op/word_dict.h"
#include "pyis/share/model_context.h"
#include "pyis/share/scope_guard.h"

using pyis::ModelContext;
using pyis::ops::WordDict;

TEST(TestWordDictCache, TestLoadFromCache) {
    ModelContext::GetActive()->ClearCache<WordDict>();
    ModelContext ctx("tests/test_share/data/not_exist_file");
    ModelContext::Activate(&ctx);
    ScopeGuard sg([&] { ModelContext::Deactivate(&ctx); });

    std::string state = R"({"version":1, "data:file":"word_dict.data.txt", "config:file":"word_dict.config.json"})";
    auto obj1 = ModelContext::GetActive()->GetOrCreateObject<WordDict>(state);
    auto obj2 = ModelContext::GetActive()->GetOrCreateObject<WordDict>(state);
    ASSERT_EQ(obj1.get(), obj2.get());

    std::string state2 =
        R"({"version":1, "data:file":"word_dict.data.copy.txt", "config:file":"word_dict.config.json"})";
}

TEST(TestWordDictCache, TestSameFileContent) {
    ModelContext::GetActive()->ClearCache<WordDict>();
    ModelContext ctx("tests/test_share/data/not_exist_file");
    ModelContext::Activate(&ctx);
    ScopeGuard sg([&] { ModelContext::Deactivate(&ctx); });

    std::string state1 = R"({"version":1, "data:file":"word_dict.data.txt", "config:file":"word_dict.config.json"})";
    auto obj1 = ModelContext::GetActive()->GetOrCreateObject<WordDict>(state1);

    std::string state2 =
        R"({"version":1, "data:file":"word_dict.data.copy.txt", "config:file":"word_dict.config.json"})";
    auto obj2 = ModelContext::GetActive()->GetOrCreateObject<WordDict>(state2);

    ASSERT_EQ(obj1.get(), obj2.get());
}

TEST(TestWordDictCache, TestUnorderedState) {
    ModelContext::GetActive()->ClearCache<WordDict>();
    ModelContext ctx("tests/test_share/data/not_exist_file");
    ModelContext::Activate(&ctx);
    ScopeGuard sg([&] { ModelContext::Deactivate(&ctx); });

    std::string state1 = R"({"version":1, "data:file":"word_dict.data.txt", "config:file":"word_dict.config.json"})";
    auto obj1 = ModelContext::GetActive()->GetOrCreateObject<WordDict>(state1);

    std::string state2 = R"({"data:file":"word_dict.data.txt", "version":1, "config:file":"word_dict.config.json"})";
    auto obj2 = ModelContext::GetActive()->GetOrCreateObject<WordDict>(state2);

    ASSERT_EQ(obj1.get(), obj2.get());
}

TEST(TestWordDictCache, TestModelContextMgmt) {
    ModelContext::GetActive()->ClearCache<WordDict>();

    // Use a scope guard(unique_ptr or shared_ptr) to guarantee deactivation of
    // model context after model loading/saving is done,
    // or the execution is interrupted by unexpected exception.
    auto deactivator = [](ModelContext* p) {
        ModelContext::Deactivate(p);
        delete p;
        std::cout << "model context deactivated" << std::endl;
    };
    std::unique_ptr<ModelContext, decltype(deactivator)> ctx(new ModelContext("tests/test_share/data/not_exist_file"),
                                                             deactivator);
    ModelContext::Activate(ctx.get());
    std::cout << "model context activated" << std::endl;

    std::string state = R"({"version":1, "data:file":"word_dict.data.txt", "config:file":"word_dict.config.json"})";
    auto obj1 = ModelContext::GetActive()->GetOrCreateObject<pyis::ops::WordDict>(state);
    auto obj2 = ModelContext::GetActive()->GetOrCreateObject<pyis::ops::WordDict>(state);
    ASSERT_EQ(obj1.get(), obj2.get());
}
