#include <codecvt>
#include <fstream>
#include <locale>

#include "gtest/gtest.h"
#include "pyis/share/binary_serialize_helper.h"

static const char TEST_FILE_NAME[] = "tmp/test_serialization.txt";
static const char TEST_FILE_CONTENT[] = "\0\1\2\3!!@#$%^&*)wertyuimbczxcvbnmsdcjsdoi1\n\r";

TEST(SerializeHelper, Basic) {
    pyis::BinarySerializeHelper serialize_helper;

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
    size_t expect_size = 1;

    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(uc);
    expect_size += sizeof(uc) + sizeof(pyis::SerializeType);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(c);
    expect_size += sizeof(c) + sizeof(pyis::SerializeType);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(us);
    expect_size += sizeof(us) + sizeof(pyis::SerializeType);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(s);
    expect_size += sizeof(s) + sizeof(pyis::SerializeType);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(ui);
    expect_size += sizeof(ui) + sizeof(pyis::SerializeType);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(i);
    expect_size += sizeof(i) + sizeof(pyis::SerializeType);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(ul);
    expect_size += sizeof(ul) + sizeof(pyis::SerializeType);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(l);
    expect_size += sizeof(l) + sizeof(pyis::SerializeType);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(f);
    expect_size += sizeof(f) + sizeof(pyis::SerializeType);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(d);
    expect_size += sizeof(d) + sizeof(pyis::SerializeType);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(str);
    expect_size += sizeof(str.size()) + str.size() + sizeof(pyis::SerializeType);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);
}

TEST(SerializeHelper, Vector) {
    pyis::BinarySerializeHelper serialize_helper;

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
    size_t expect_size = 1;

    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(uc);
    expect_size +=
        sizeof(uc[0]) * uc.size() + sizeof(pyis::SerializeType) + sizeof(pyis::SerializeType) + sizeof(size_t);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(c);
    expect_size += sizeof(c[0]) * c.size() + sizeof(pyis::SerializeType) + sizeof(pyis::SerializeType) + sizeof(size_t);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(us);
    expect_size +=
        sizeof(us[0]) * us.size() + sizeof(pyis::SerializeType) + sizeof(pyis::SerializeType) + sizeof(size_t);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(s);
    expect_size += sizeof(s[0]) * s.size() + sizeof(pyis::SerializeType) + sizeof(pyis::SerializeType) + sizeof(size_t);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(ui);
    expect_size +=
        sizeof(ui[0]) * ui.size() + sizeof(pyis::SerializeType) + sizeof(pyis::SerializeType) + sizeof(size_t);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(i);
    expect_size += sizeof(i[0]) * i.size() + sizeof(pyis::SerializeType) + sizeof(pyis::SerializeType) + sizeof(size_t);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(ul);
    expect_size +=
        sizeof(ul[0]) * ul.size() + sizeof(pyis::SerializeType) + sizeof(pyis::SerializeType) + sizeof(size_t);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(l);
    expect_size += sizeof(l[0]) * l.size() + sizeof(pyis::SerializeType) + sizeof(pyis::SerializeType) + sizeof(size_t);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(f);
    expect_size += sizeof(f[0]) * f.size() + sizeof(pyis::SerializeType) + sizeof(pyis::SerializeType) + sizeof(size_t);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(d);
    expect_size += sizeof(d[0]) * d.size() + sizeof(pyis::SerializeType) + sizeof(pyis::SerializeType) + sizeof(size_t);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);

    serialize_helper.add(str);
    expect_size +=
        sizeof(size_t) + str[0].size() + sizeof(pyis::SerializeType) + sizeof(pyis::SerializeType) + sizeof(size_t);
    ASSERT_TRUE(serialize_helper.serialize().length() == expect_size);
}

TEST(SerializeHelper, FILE) {
    system("mkdir tmp");

    std::ofstream test_file(TEST_FILE_NAME, std::ios::binary | std::ios::out);
    test_file.write(TEST_FILE_CONTENT, sizeof(TEST_FILE_CONTENT));
    test_file.close();

    pyis::BinarySerializeHelper helper;
    helper.add_file(TEST_FILE_NAME);

    ASSERT_EQ(helper.serialize().length(),
              sizeof(TEST_FILE_CONTENT) + sizeof(size_t) + sizeof(pyis::SerializeType) + sizeof(uint8_t));

    remove(TEST_FILE_NAME);
}