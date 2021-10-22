#include <codecvt>
#include <fstream>
#include <locale>

#include "gtest/gtest.h"
#include "pyis/share/binary_deserialize_helper.h"
#include "pyis/share/binary_serialize_helper.h"

static const char TEST_FILE_NAME[] = "tmp/test_serialization.txt";
static const char OUTPUT_TEST_FILE_NAME[] = "tmp/test_serialization.txt.output";
static const char TEST_FILE_CONTENT[] = "\0\1\2\3!!@#$%^&*)wertyuimbczxcvbnmsdcjsdoi1\n\r";

TEST(DeSerializeHelper, Basic) {
    uint8_t uc = 1;
    int8_t c = 2;
    uint16_t us = 3;
    int16_t s = 4;
    uint32_t ui = 5;
    int32_t i = 6;
    uint64_t ul = 7;
    int64_t l = 8;
    float f = 9.0;
    double d = 10.0;
    std::string str = "abcdefg";

    pyis::BinarySerializeHelper serialize_helper;
    serialize_helper.add(uc).add(c).add(us).add(s).add(ui).add(i).add(ul).add(l).add(f).add(d).add(str);
    pyis::BinaryDeserializeHelper deserialize_helper(serialize_helper.serialize());

    uint8_t r_uc;
    deserialize_helper.get(r_uc);
    ASSERT_EQ(uc, r_uc);

    int8_t r_c;
    deserialize_helper.get(r_c);
    ASSERT_EQ(c, r_c);

    uint16_t r_us;
    deserialize_helper.get(r_us);
    ASSERT_EQ(us, r_us);

    int16_t r_s;
    deserialize_helper.get(r_s);
    ASSERT_EQ(s, r_s);

    uint32_t r_ui;
    deserialize_helper.get(r_ui);
    ASSERT_EQ(ui, r_ui);

    int32_t r_i;
    deserialize_helper.get(r_i);
    ASSERT_EQ(i, r_i);

    uint64_t r_ul;
    deserialize_helper.get(r_ul);
    ASSERT_EQ(ul, r_ul);

    int64_t r_l;
    deserialize_helper.get(r_l);
    ASSERT_EQ(l, r_l);

    float r_f;
    deserialize_helper.get(r_f);
    ASSERT_EQ(f, r_f);

    double r_d;
    deserialize_helper.get(r_d);
    ASSERT_EQ(d, r_d);

    std::string r_str;
    deserialize_helper.get(r_str);
    ASSERT_EQ(r_str, str);
}

TEST(DeSerializeHelper, Vector) {
    std::vector<uint8_t> uc = {1, 2, 3};
    std::vector<int8_t> c = {1, 2, 3};
    std::vector<uint16_t> us = {1, 2, 3};
    std::vector<int16_t> s = {1, 3, 3};
    std::vector<uint32_t> ui = {1, 2, 3};
    std::vector<int32_t> i = {1, 2, 3};
    std::vector<uint64_t> ul = {1, 2, 3};
    std::vector<int64_t> l = {1, 2, 3};
    std::vector<float> f = {1.0, 2.0, 3.0};
    std::vector<double> d = {1.0, 2.0, 3.0};
    std::vector<std::string> str = {"abcdefg"};

    pyis::BinarySerializeHelper serialize_helper;
    serialize_helper.add(uc).add(c).add(us).add(s).add(ui).add(i).add(ul).add(l).add(f).add(d).add(str);
    pyis::BinaryDeserializeHelper deserialize_helper(serialize_helper.serialize());

    std::vector<uint8_t> r_uc;
    deserialize_helper.get(r_uc);
    ASSERT_EQ(uc, r_uc);

    std::vector<int8_t> r_c;
    deserialize_helper.get(r_c);
    ASSERT_EQ(c, r_c);

    std::vector<uint16_t> r_us;
    deserialize_helper.get(r_us);
    ASSERT_EQ(us, r_us);

    std::vector<int16_t> r_s;
    deserialize_helper.get(r_s);
    ASSERT_EQ(s, r_s);

    std::vector<uint32_t> r_ui;
    deserialize_helper.get(r_ui);
    ASSERT_EQ(ui, r_ui);

    std::vector<int32_t> r_i;
    deserialize_helper.get(r_i);
    ASSERT_EQ(i, r_i);

    std::vector<uint64_t> r_ul;
    deserialize_helper.get(r_ul);
    ASSERT_EQ(ul, r_ul);

    std::vector<int64_t> r_l;
    deserialize_helper.get(r_l);
    ASSERT_EQ(l, r_l);

    std::vector<float> r_f;
    deserialize_helper.get(r_f);
    ASSERT_EQ(f, r_f);

    std::vector<double> r_d;
    deserialize_helper.get(r_d);
    ASSERT_EQ(d, r_d);

    std::vector<std::string> r_str;
    deserialize_helper.get(r_str);
    ASSERT_EQ(r_str, str);
}

TEST(DeSerializeHelper, File) {
    system("mkdir tmp");

    std::ofstream test_file(TEST_FILE_NAME, std::ios::binary | std::ios::out);
    test_file.write(TEST_FILE_CONTENT, sizeof(TEST_FILE_CONTENT));
    test_file.close();

    pyis::BinarySerializeHelper helper;
    helper.add_file(TEST_FILE_NAME);

    pyis::BinaryDeserializeHelper test_1(helper.serialize());
    std::string output;
    test_1.get_file_content(output);
    ASSERT_EQ(output, std::string(TEST_FILE_CONTENT, sizeof(TEST_FILE_CONTENT)));

    pyis::BinaryDeserializeHelper test_2(helper.serialize());
    test_2.get_file(OUTPUT_TEST_FILE_NAME);
    std::ifstream output_file(OUTPUT_TEST_FILE_NAME, std::ios::binary | std::ios::in);

    std::string output_file_content;
    output_file_content.resize(sizeof(TEST_FILE_CONTENT));
    output_file.read(&output_file_content[0], sizeof(TEST_FILE_CONTENT));
    ASSERT_EQ(output_file_content, std::string(TEST_FILE_CONTENT, sizeof(TEST_FILE_CONTENT)));

    // ensure no more chars in the file.
    std::string end;
    output_file >> end;
    ASSERT_TRUE(output_file.eof());
    ASSERT_TRUE(end.empty());

    remove(TEST_FILE_NAME);
    remove(OUTPUT_TEST_FILE_NAME);
}