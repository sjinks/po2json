#include <sstream>
#include <string>
#include <gtest/gtest.h>
#include "gettext.h"
#include "processor.h"

namespace {

TEST(ProcessorTest, process_file_empty)
{
    const std::string expected{
        R"({"domain":"messages","locale_data":{"messages":{"":{"domain":"messages","plural_forms":"","lang":""}}}})"
        "\n"};

    const po::file po;
    std::ostringstream json;
    process_file(po, json);

    const std::string actual = json.str();
    EXPECT_EQ(actual, expected);
}

TEST(ProcessorTest, process_file_normal)
{
    const std::string expected{
        R"({"domain":"domain","locale_data":{"domain":{"":{"domain":"domain","plural_forms":"","lang":"en"},"context\u0004singular":["translation"],"context\u0004plural_s":["translation1","translation2"]}}})"
        "\n"};

    const po::file po;
    auto it = po.message_iterator("domain");
    po::message::create(it, "", "Language: en");
    po::message::create(it, "singular", "translation", "context");
    po::message::create_plural(it, "plural_s", "plural_p", {{"translation1"}, {"translation2"}}, "context");

    std::ostringstream json;
    process_file(po, json);

    const std::string actual = json.str();
    EXPECT_EQ(actual, expected);
}

}  // namespace
