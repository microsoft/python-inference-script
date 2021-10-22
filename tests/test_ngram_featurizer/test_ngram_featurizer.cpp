#include "gtest/gtest.h"
#include "pyis/ops/text/ngram_featurizer.h"

TEST(TestNGramFeaturizer, TestLoad) {
    std::string ngram_file = "tests/test_ngram_featurizer/data/ngram.txt";
    pyis::ops::NGramFeaturizer featurizer(1, false);
    featurizer.LoadNGram(ngram_file);

    auto features = featurizer.Transform({"hello", "world"});
    ASSERT_EQ(features.size(), 2);
    ASSERT_EQ(features[0].id(), 0);
    ASSERT_EQ(features[1].id(), 1);
}