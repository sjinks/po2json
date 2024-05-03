#ifndef DF6EB2B9_0D0F_4DA6_B6C1_CAED6D2D79FB
#define DF6EB2B9_0D0F_4DA6_B6C1_CAED6D2D79FB

#include <cstdlib>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <type_traits>
#include <vector>
#include <gettext-po.h>

namespace po {

class header;
class message;
class message_iterator;

class exception : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class file {
public:
    file();
    explicit file(const char* filename);

    [[nodiscard]] std::span<const char* const> domains() const;
    [[nodiscard]] header domain_header(const char* domain) const;
    [[nodiscard]] po::message_iterator message_iterator(const char* domain) const;

private:
    static void free_file(po_file_t file);

    std::unique_ptr<std::remove_pointer_t<po_file_t>, decltype(&file::free_file)> m_file;
};

class header {
public:
    explicit header(const char* header);

    [[nodiscard]] bool empty() const;
    [[nodiscard]] std::string field(const char* field) const;

private:
    std::string m_header;
};

class message_iterator {
public:
    friend class file;

    [[nodiscard]] std::optional<message> next() const;
    void insert(const message& msg);

private:
    std::unique_ptr<std::remove_pointer_t<po_message_iterator_t>, decltype(&po_message_iterator_free)> m_iterator;

    message_iterator(po_file_t file, const char* domain);
};

class message {
public:
    friend class message_iterator;

    static message
    create(message_iterator& it, const std::string& msgid, const std::string& msgstr, const std::string& msgctx = {});

    static message create_plural(
        message_iterator& it,
        const std::string& msgid,
        const std::string& msgid_plural,
        const std::vector<std::string>& msgstr_plural,
        const std::string& msgctx = {}
    );

    [[nodiscard]] const char* msgid() const;
    [[nodiscard]] const char* msgid_plural() const;
    [[nodiscard]] const char* msgctx() const;
    [[nodiscard]] std::string key() const;
    [[nodiscard]] std::string msgstr() const;
    [[nodiscard]] std::vector<std::string> msgstr_plural() const;

private:
    po_message_t m_message;

    explicit message(po_message_t message);
};

}  // namespace po

#endif /* DF6EB2B9_0D0F_4DA6_B6C1_CAED6D2D79FB */
