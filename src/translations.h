#ifndef D3E4015A_C6B8_4ABC_8626_A39AECD4AC75
#define D3E4015A_C6B8_4ABC_8626_A39AECD4AC75

#include <optional>
#include <string>
#include <vector>

struct header_t {
    std::string domain;
    std::string plural_forms;
    std::string lang;
    std::string charset;

    bool operator==(const header_t& other) const = default;
};

struct translations_t {
    std::string key;
    std::vector<std::string> translations;
};

namespace po {
class header;
class message;
}  // namespace po

header_t process_header(const po::header& header, const char* domain);
std::optional<translations_t> process_message(const po::message& message);

#endif /* D3E4015A_C6B8_4ABC_8626_A39AECD4AC75 */
