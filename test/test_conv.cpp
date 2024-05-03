#include <system_error>
#include <gtest/gtest.h>
#include "conv.h"

namespace {

class ConvTest : public ::testing::Test {
public:
    ConvTest() = default;

protected:
    converter_iconv m_converter;

    void SetUp() override { this->m_converter.set_charsets("UTF-8", "UTF-8"); }
};

TEST_F(ConvTest, same_charset)
{
    const std::string input{"\x00\x01\x02\x03\x04", 5};

    EXPECT_EQ(this->m_converter(input), input);
}

TEST_F(ConvTest, two_way_convert)
{
    const std::string input{"путин-хуйло"};
    const std::string expected1{"\xD0\xD5\xD4\xC9\xCE-\xC8\xD5\xCA\xCC\xCF"};

    EXPECT_NO_THROW(this->m_converter.set_charsets("UTF-8", "KOI8-U"));
    const auto actual1 = this->m_converter(input);
    ASSERT_EQ(actual1, expected1);

    EXPECT_NO_THROW(this->m_converter.set_charsets("KOI8-U", "UTF-8"));
    const auto actual2 = this->m_converter(actual1);
    ASSERT_EQ(actual2, input);
}

TEST_F(ConvTest, ignore_invalid)
{
    const std::string input{"утіпутін"};
    const std::string expected{"\xD5\xD4\xD0\xD5\xD4\xCE"};

    EXPECT_NO_THROW(this->m_converter.set_charsets("UTF-8", "KOI8-R//IGNORE"));
    const auto actual = this->m_converter(input);
    EXPECT_EQ(actual, expected);
}

TEST_F(ConvTest, throw_invalid)
{
    const std::string input{"путін"};
    EXPECT_NO_THROW(this->m_converter.set_charsets("UTF-8", "KOI8-R"));
    EXPECT_THROW(this->m_converter(input), std::system_error);
}

TEST_F(ConvTest, empty_string)
{
    const std::string input;
    EXPECT_EQ(this->m_converter(input), input);
}

TEST_F(ConvTest, invalid_charset)
{
    const std::string input{"путін"};
    EXPECT_THROW(this->m_converter.set_charsets("UTF-8", "KOI8-RR"), std::system_error);
}

TEST_F(ConvTest, e2big)
{
    const std::string input{"\xD0\xD0\xD0\xD0"};
    const std::string expected{"\xFF\xFE\x00\x00\x3F\x04\x00\x00\x3F\x04\x00\x00\x3F\x04\x00\x00\x3F\x04\x00\x00", 20};
    EXPECT_NO_THROW(this->m_converter.set_charsets("KOI8-U", "UTF-32"));
    EXPECT_EQ(this->m_converter(input), expected);
}

}  // namespace
