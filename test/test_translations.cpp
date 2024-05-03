#include <cstring>
#include <gtest/gtest.h>
#include "gettext.h"
#include "translations.h"

namespace {

TEST(TranslationsTest, process_header)
{
    const char* input{"PO-Revision-Date: 2024-03-23 15:40:17+0000\n"
                      "MIME-Version: 1.0\n"
                      "Content-Type: text/plain; charset=UTF-8\n"
                      "Content-Transfer-Encoding: 8bit\n"
                      "Plural-Forms: nplurals=3; plural=(n % 10 == 1 && n % 100 != 11) ? 0 : ((n % 10 >= 2 && n % "
                      "10 <= 4 && (n % 100 < 12 || n % 100 > 14)) ? 1 : 2);\n"
                      "X-Generator: GlotPress/4.0.1\n"
                      "Language: ru\n"
                      "Project-Id-Version: WordPress - 6.5.x - Development - Administration\n"};

    const header_t expected{
        "messages",
        "nplurals=3; plural=(n % 10 == 1 && n % 100 != 11) ? 0 : ((n % 10 >= 2 && n % 10 <= 4 && (n % 100 < 12 || n % "
        "100 > 14)) ? 1 : 2);",
        "ru",
        "UTF-8",
    };

    const po::header header{input};

    const header_t actual = process_header(header, "messages");
    EXPECT_EQ(expected, actual);
}

TEST(TranslationsTest, process_header_charset_extra_chars)
{
    const char* input{"Content-Type: text/plain; charset=KOI8-U; someattribute\n"};
    const header_t expected{"messages", "", "", "KOI8-U"};

    const po::header header{input};

    const header_t actual = process_header(header, "messages");
    EXPECT_EQ(expected, actual);
}

TEST(TranslationsTest, process_header_no_charset)
{
    const char* input{"MIME-Version: 1.0\nProject-Id-Version: WordPress - 6.5.x - Development - Administration\n"};
    const header_t expected{"messages", "", "", "UTF-8"};

    const po::header header{input};

    const header_t actual = process_header(header, "messages");
    EXPECT_EQ(expected, actual);
}

TEST(TranslationsTest, process_header_empty)
{
    const header_t expected{"domain", "", "", "UTF-8"};
    const po::header header{nullptr};

    const header_t actual = process_header(header, "domain");
    EXPECT_EQ(expected, actual);
}

TEST(TranslationsTest, process_message_header)
{
    const po::file po;
    auto it      = po.message_iterator("messages");
    auto message = po::message::create(it, "", "Header");

    auto actual = process_message(message);
    EXPECT_FALSE(actual.has_value());
}

TEST(TranslationsTest, process_message_singular)
{
    const std::string key{"singular"};
    const std::string translation{"translation"};

    const po::file po;
    auto it      = po.message_iterator("messages");
    auto message = po::message::create(it, key, translation);

    auto actual = process_message(message);
    ASSERT_TRUE(actual.has_value());

    if (actual.has_value()) {  // to keep clang-tidy happy
        EXPECT_EQ(actual->key, key);
        EXPECT_EQ(actual->translations.size(), 1);
        EXPECT_EQ(actual->translations[0], translation);
    }
}

TEST(TranslationsTest, process_message_plural)
{
    const std::string id{"plural"};
    const std::string id_plural{"plural!"};
    const std::string ctx{"ctx"};
    const std::vector<std::string> translations{{"translation1"}, {"translation2"}};

    const std::string expected_key = ctx + '\x04' + id;

    const po::file po;
    auto it      = po.message_iterator("messages");
    auto message = po::message::create_plural(it, id, id_plural, translations, ctx);

    auto actual = process_message(message);
    ASSERT_TRUE(actual.has_value());

    if (actual.has_value()) {  // to keep clang-tidy happy
        EXPECT_EQ(actual->key, expected_key);
        EXPECT_EQ(actual->translations, translations);
    }
}

}  // namespace
