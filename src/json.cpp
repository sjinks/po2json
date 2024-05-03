#include "json.h"
#include <format>
#include <sstream>

std::string jstring(const std::string& s)
{
    std::ostringstream o;
    o << '"';
    for (const auto c : s) {
        switch (c) {
            case '"':
                o << R"(\")";
                break;
            case '\\':
                o << R"(\\)";
                break;
            case '\b':
                o << "\\b";
                break;
            case '\f':
                o << "\\f";
                break;
            case '\n':
                o << "\\n";
                break;
            case '\r':
                o << "\\r";
                break;
            case '\t':
                o << "\\t";
                break;
            default:
                if (static_cast<unsigned char>(c) < ' ') {
                    o << std::format("\\u{:04x}", static_cast<int>(c));
                }
                else {
                    o << c;
                }
        }
    }

    o << '"';
    return o.str();
}
