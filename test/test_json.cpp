#include <format>
#include <gtest/gtest.h>
#include "json.h"

namespace {

TEST(JsonTest, ascii_string)
{
    const std::string input{"Glory to Ukraine"};
    const std::string expected{R"("Glory to Ukraine")"};
    const auto actual = jstring(input);
    EXPECT_EQ(actual, expected);
}

TEST(JsonTest, utf_string)
{
    const std::string input{"Слава Україні!"};
    const std::string expected{R"("Слава Україні!")"};
    const auto actual = jstring(input);
    EXPECT_EQ(actual, expected);
}

TEST(JsonTest, special_chars)
{
    const std::string input{"\"\\/\b\f\n\r\t"};
    const std::string expected{R"("\"\\/\b\f\n\r\t")"};
    const auto actual = jstring(input);
    EXPECT_EQ(actual, expected);
}

TEST(JsonTest, control_chars)
{
    std::string input;
    std::string expected;

    input.reserve(32 - 5);
    expected.reserve((32 - 5) * 6 + 2);

    expected.push_back('"');
    for (char c = 0; c < ' '; ++c) {
        if (c != '\b' && c != '\f' && c != '\n' && c != '\r' && c != '\t') {
            input.push_back(c);
            expected += std::format("\\u{:04x}", static_cast<int>(c));
        }
    }
    expected.push_back('"');

    const auto actual = jstring(input);
    EXPECT_EQ(actual, expected);
}

}  // namespace
