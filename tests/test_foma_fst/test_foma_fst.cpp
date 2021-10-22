#include <climits>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <string>

#include "gtest/gtest.h"
#include "pyis/ops/text/foma_fst.h"
TEST(TestFomaFst, Basic) {
    pyis::ops::FomaFst fst("tests/test_foma_fst/data/recognize_latin.fst");

    ASSERT_EQ(fst.ApplyDown("avoid writing ad hoc code"), "avoid writing <latin>ad hoc</latin> code");
}

TEST(TestFomaFst, Serilize) {
    system("mkdir tmp");
    pyis::ops::FomaFst fst("tests/test_foma_fst/data/recognize_latin.fst");
    ASSERT_EQ(fst.ApplyDown("avoid writing ad hoc code"), "avoid writing <latin>ad hoc</latin> code");
    std::ofstream fout("tmp/FomaFstSerilize.fst", std::ios::binary);
    fst.SaveBinary(fout);
    fout.close();
    std::ifstream fin = std::ifstream("tmp/FomaFstSerilize.fst", std::ios::binary);
    fst.LoadBinary(fin);
    fin.close();
    ASSERT_EQ(fst.ApplyDown("avoid writing ad hoc code"), "avoid writing <latin>ad hoc</latin> code");
}

TEST(TestFomaFst, CompileFromStr) {
    system("mkdir tmp");
    pyis::ops::FomaFst::CompileFromStr(R""""(regex "ad hoc" -> %<latin%> ... %<%/latin%> ;
apply down avoid writing ad hoc code
save stack tmp/FomaFstCompileFromStr.fst )"""");
    pyis::ops::FomaFst fst("tmp/FomaFstCompileFromStr.fst");
    ASSERT_EQ(fst.ApplyDown("avoid writing ad hoc code"), "avoid writing <latin>ad hoc</latin> code");
    // run second time to test memory leak
    pyis::ops::FomaFst::CompileFromStr(R""""(regex "ad hoc" -> %<latin%> ... %<%/latin%> ;
apply down avoid writing ad hoc code
save stack tmp/FomaFstCompileFromStr.fst )"""");
}
