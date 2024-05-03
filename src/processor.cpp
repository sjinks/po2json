#include "processor.h"
#include <format>
#include "conv.h"
#include "gettext.h"
#include "json.h"
#include "translations.h"

void process_messages(const po::message_iterator& iterator, std::ostream& json, const converter_iconv& converter)
{
    for (auto message = iterator.next(); message.has_value(); message = iterator.next()) {
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        if (const auto translations = process_message(message.value()); translations.has_value()) {
            const auto& xlations = translations.value();
            json << std::format(R"(,{}:[)", jstring(xlations.key));

            bool first = true;
            for (const auto& translation : xlations.translations) {
                json << std::format("{}{}", first ? "" : ",", jstring(converter(translation)));
                first = false;
            }

            json << ']';
        }
    }
}

void process_file(const po::file& po, std::ostream& json)
{
    converter_iconv converter;
    bool has_domain = false;
    bool first      = true;
    for (const auto domains = po.domains(); const auto* domain : domains) {
        auto jdomain = jstring(domain != nullptr ? domain : "messages");
        if (!has_domain) {
            const auto it = po.message_iterator(domain);
            if (const auto msg = it.next(); !msg.has_value()) {
                continue;
            }

            json << std::format(R"({{"domain":{},"locale_data":{{)", jdomain);
            has_domain = true;
        }

        json << std::format(R"({}{}:{{)", first ? "" : ",", jdomain);
        first = false;

        auto po_header = po.domain_header(domain);
        auto header    = process_header(po_header, domain);

        json << std::format(
            R"("":{{"domain":{},"plural_forms":{},"lang":{}}})",
            jdomain,
            jstring(header.plural_forms),
            jstring(header.lang)
        );

        converter.set_charsets(header.charset, "UTF-8");
        auto iterator = po.message_iterator(domain);
        process_messages(iterator, json, converter);

        json << '}';
    }

    if (!has_domain) {
        json
            << R"({"domain":"messages","locale_data":{"messages":{"":{"domain":"messages","plural_forms":"","lang":""}}}})"
               "\n";
    }
    else {
        json << "}}\n";
    }
}
