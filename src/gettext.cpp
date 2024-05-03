#include "gettext.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <gettext-po.h>

namespace {

// LCOV_EXCL_START
std::string location(const char* filename, std::size_t lineno, std::size_t column)
{
    std::string loc;
    if (filename != nullptr) {
        loc.append(filename);
        if (lineno != static_cast<std::size_t>(-1)) {
            loc.append(1, ':').append(std::to_string(lineno));
            if (column != static_cast<std::size_t>(-1)) {
                loc.append(1, ':').append(std::to_string(column));
            }
        }

        loc.append(": ");
    }

    return loc;
}

std::string get_severity(int severity)
{
    switch (severity) {
        case PO_SEVERITY_WARNING:
            return "warning: ";
        case PO_SEVERITY_ERROR:
            return "error: ";
        case PO_SEVERITY_FATAL_ERROR:
            return "fatal error: ";
        default:
            return "";
    }
}

void xerror(
    int severity,
    po_message_t,
    const char* filename,
    std::size_t lineno,
    std::size_t column,
    int,
    const char* message_text
)
{
    std::cerr << location(filename, lineno, column) << get_severity(severity) << message_text << '\n';
    if (severity == PO_SEVERITY_FATAL_ERROR) {
        std::exit(EXIT_FAILURE);  // NOLINT(concurrency-mt-unsafe)
    }
}

void xerror2(
    int severity,
    po_message_t,
    const char* filename1,
    std::size_t lineno1,
    std::size_t column1,
    int,
    const char* message_text1,
    po_message_t,
    const char* filename2,
    std::size_t lineno2,
    std::size_t column2,
    int,
    const char* message_text2
)
{
    std::cerr << location(filename1, lineno1, column1) << get_severity(severity) << message_text1 << "...\n"
              << location(filename2, lineno2, column2) << "..." << message_text2 << '\n';

    if (severity == PO_SEVERITY_FATAL_ERROR) {
        std::exit(EXIT_FAILURE);  // NOLINT(concurrency-mt-unsafe)
    }
}

// LCOV_EXCL_STOP

void free_pchar(char* p)
{
    // NOLINTNEXTLINE(*-no-malloc, cppcoreguidelines-owning-memory)
    std::free(p);
}

const struct po_xerror_handler xerror_handler = {xerror, xerror2};

}  // namespace

po::file::file() : m_file(po_file_create(), &po::file::free_file)
{
    // LCOV_EXCL_START -- `po_file_create()` does not seem to return `NULL`
    if (!this->m_file) {
        throw po::exception("Failed to create a new PO file");
    }
    // LCOV_EXCL_STOP
}

po::file::file(const char* filename) : m_file(po_file_read(filename, &xerror_handler), &po::file::free_file)
{
    if (!this->m_file) {
        throw std::invalid_argument("Failed to open " + std::string(filename));
    }
}

std::span<const char* const> po::file::domains() const
{
    const auto* d = po_file_domains(this->m_file.get());
    std::size_t n = 0;

    for (const auto* p = d; *p != nullptr; ++p) {  // NOLINT‌(*-pointer-arithmetic)
        ++n;
    }

    return {d, n};
}

po::header po::file::domain_header(const char* domain) const
{
    /**
     * `po_file_domain_header()` uses `xstrdup()` to allocate memory for the header.
     * The memory is not reclaimed by any of libgettextpo's functions.
     *
     * ==848234== 75 bytes in 1 blocks are definitely lost in loss record 2 of 4
     * ==848234==    at 0x4845828: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
     * ==848234==    by 0x48ADB2C: libgettextpo_xmalloc (in /usr/lib/x86_64-linux-gnu/libgettextpo.so.0.5.7)
     * ==848234==    by 0x48AE105: libgettextpo_xstrdup (in /usr/lib/x86_64-linux-gnu/libgettextpo.so.0.5.7)
     * ==848234==    by 0x1100F4: po::file::domain_header(char const*) const (gettext.cpp:89)
     *
     * We have to do `const_cast` to remove the `const` qualifier and free the memory ourselves.
     */
    const std::unique_ptr<char, decltype(&free_pchar)> header(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        const_cast<char*>(po_file_domain_header(this->m_file.get(), domain)),
        &free_pchar
    );

    return po::header(header.get());
}

po::message_iterator po::file::message_iterator(const char* domain) const
{
    return {this->m_file.get(), domain};
}

void po::file::free_file(po_file_t file)
{
    if (file != nullptr) {
        ::po_file_free(file);
    }
}

po::header::header(const char* header)
{
    if (header != nullptr) {
        this->m_header = header;
    }
}

bool po::header::empty() const
{
    return this->m_header.empty();
}

std::string po::header::field(const char* field) const
{
    if (field != nullptr) {
        const std::unique_ptr<char, decltype(&free_pchar)> value(
            po_header_field(this->m_header.c_str(), field), &free_pchar
        );

        if (value != nullptr) {
            return value.get();
        }
    }

    return {};
}

std::optional<po::message> po::message_iterator::next() const
{
    if (auto* message = po_next_message(this->m_iterator.get()); message) {
        return po::message(message);
    }

    return std::nullopt;
}

void po::message_iterator::insert(const po::message& msg)
{
    po_message_insert(this->m_iterator.get(), msg.m_message);
}

po::message_iterator::message_iterator(po_file_t file, const char* domain)
    : m_iterator(::po_message_iterator(file, domain), &po_message_iterator_free)
{}

po::message po::message::create(
    message_iterator& it, const std::string& msgid, const std::string& msgstr, const std::string& msgctx
)
{
    po_message_t msg = po_message_create();
    // LCOV_EXCL_START -- should have an OOM to test this?
    if (msg == nullptr) {
        throw po::exception("Failed to create a new PO message");
    }
    // LCOV_EXCL_STOP

    po_message_set_msgid(msg, msgid.c_str());
    po_message_set_msgstr(msg, msgstr.c_str());
    if (!msgctx.empty()) {
        po_message_set_msgctxt(msg, msgctx.c_str());
    }

    auto result = po::message(msg);
    it.insert(result);
    return result;
}

po::message po::message::create_plural(
    message_iterator& it,
    const std::string& msgid,
    const std::string& msgid_plural,
    const std::vector<std::string>& msgstr_plural,
    const std::string& msgctx
)
{
    po_message_t msg = po_message_create();
    // LCOV_EXCL_START -- should have an OOM to test this?
    if (msg == nullptr) {
        throw po::exception("Failed to create a new PO message");
    }
    // LCOV_EXCL_STOP

    po_message_set_msgid(msg, msgid.c_str());
    po_message_set_msgid_plural(msg, msgid_plural.c_str());
    for (std::size_t i = 0; i < msgstr_plural.size(); ++i) {
        po_message_set_msgstr_plural(msg, static_cast<int>(i), msgstr_plural[i].c_str());
    }

    if (!msgctx.empty()) {
        po_message_set_msgctxt(msg, msgctx.c_str());
    }

    auto result = po::message(msg);
    it.insert(result);
    return result;
}

const char* po::message::msgid() const
{
    return po_message_msgid(this->m_message);
}

const char* po::message::msgid_plural() const
{
    return po_message_msgid_plural(this->m_message);
}

const char* po::message::msgctx() const
{
    return po_message_msgctxt(this->m_message);
}

std::string po::message::key() const
{
    const auto* ctx = this->msgctx();
    const auto* id  = this->msgid();
    if (ctx != nullptr) {
        return std::string(ctx) + '\x04' + id;
    }

    return id;
}

std::string po::message::msgstr() const
{
    const auto* s = po_message_msgstr(this->m_message);
    return s != nullptr ? s : "";
}

std::vector<std::string> po::message::msgstr_plural() const
{
    std::vector<std::string> result;
    result.reserve(2);

    for (int i = 0;; ++i) {
        const auto* msgstr_plural = po_message_msgstr_plural(this->m_message, i);
        if (msgstr_plural == nullptr) {
            break;
        }

        result.emplace_back(msgstr_plural);
    }

    return result;
}

po::message::message(po_message_t message) : m_message(message) {}
