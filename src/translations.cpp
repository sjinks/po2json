#include "translations.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <ranges>
#include "gettext.h"

namespace {

std::string extract_charset(const po::header& hdr)
{
    static const char* charset_key = "charset=";

    if (!hdr.empty()) {
        const auto header = hdr.field("Content-Type");

        if (const auto pos = header.find(charset_key); pos != std::string::npos) {
            const auto start_pos = pos + std::strlen(charset_key);
            auto end_pos =
                header.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-", start_pos);
            if (end_pos == std::string::npos) {
                end_pos = header.length();
            }

            std::string charset(&header[start_pos], &header[end_pos]);
            std::ranges::transform(charset, charset.begin(), ::toupper);
            if (!charset.empty()) {
                return charset;
            }
        }
    }

    return {"UTF-8"};
}

}  // namespace

header_t process_header(const po::header& header, const char* domain)
{
    return {
        domain != nullptr ? domain : "messages",
        header.field("Plural-Forms"),
        header.field("Language"),
        extract_charset(header),
    };
}

std::optional<translations_t> process_message(const po::message& message)
{
    translations_t result;

    if (const auto* msgid = message.msgid(); std::strlen(msgid) != 0) {
        if (const auto* msgid_plural = message.msgid_plural(); msgid_plural) {
            result.translations = message.msgstr_plural();
        }
        else {
            result.translations.push_back(message.msgstr());
        }

        result.key = message.key();
        return result;
    }

    return {};
}
