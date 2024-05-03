#include "conv.h"
#include <bit>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <system_error>
#include <type_traits>
#include <vector>

#include <iostream>

namespace {

template<typename T>
constexpr std::intptr_t intptr_cast(T ptr) noexcept
{
    return std::bit_cast<std::intptr_t>(ptr);
}

iconv_t iconv_open(const std::string& to, const std::string& from)
{
    iconv_t result = ::iconv_open(to.c_str(), from.c_str());
    if (intptr_cast(result) == -1) {
        if (errno == EINVAL) {
            auto pos = to.find("//IGNORE");
            if (pos != std::string::npos) {
                // LCOV_EXCL_START -- this will run under non-GNU iconv (like musl)
                std::string new_to(to);
                new_to.erase(pos, std::strlen("//IGNORE"));
                result = ::iconv_open(new_to.c_str(), from.c_str());
            }
            // LCOV_EXCL_STOP
        }

        if (intptr_cast(result) == -1) {
            throw std::system_error(errno, std::system_category(), "iconv_open");
        }
    }

    return result;
}

std::size_t handle_ilseq(std::size_t& inbytes, char*& inbuf)
{
    if (inbytes <= 1) {
        return 0;
    }

    // LCOV_EXCL_START -- this will run under non-GNU iconv (like musl)
    ++inbuf;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    --inbytes;
    return static_cast<std::size_t>(-1);
    // LCOV_EXCL_STOP
}

void handle_e2big(std::vector<char>& output, char*& outbuf, std::size_t& outbytes)
{
    const std::size_t used = outbuf - output.data();
    // NOLINTNEXTLINE(*-magic-numbers)
    output.resize(output.size() + 32);
    outbuf   = &output[used];
    outbytes = output.size() - used;
}

void handle_tail(iconv_t cd, std::vector<char>& output, char*& outbuf, std::size_t& outbytes)
{
    std::size_t result = 0;
    do {
        result = iconv(cd, nullptr, nullptr, &outbuf, &outbytes);
        if (result == static_cast<std::size_t>(-1)) {
            if (errno != E2BIG) {
                throw std::system_error(errno, std::system_category(), "iconv");
            }

            handle_e2big(output, outbuf, outbytes);
        }
    } while (result == static_cast<std::size_t>(-1));
}

}  // namespace

converter_iconv::converter_iconv() : m_cd(nullptr, &converter_iconv::free_iconv) {}

void converter_iconv::set_charsets(const std::string& from, const std::string& to)
{
    if (from == to) {
        this->m_cd.reset();
        this->m_ignore = false;
    }
    else {
        this->m_cd.reset(iconv_open(to, from));
        this->m_ignore = to.find("//IGNORE") != std::string::npos;
    }
}

std::string converter_iconv::operator()(const std::string& str) const
{
    auto* cd = this->m_cd.get();
    if (str.empty() || cd == nullptr) {
        return str;
    }

    std::size_t inbytes  = str.size();
    std::size_t outbytes = 3 * inbytes;

    std::vector<char> output(outbytes);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    auto* inbuf  = const_cast<char*>(str.data());
    char* outbuf = output.data();

    std::size_t result = 0;
    while (inbytes > 0) {
        result = iconv(cd, &inbuf, &inbytes, &outbuf, &outbytes);
        if (result == static_cast<std::size_t>(-1)) {
            if (this->m_ignore && errno == EILSEQ) {
                result = handle_ilseq(inbytes, inbuf);
            }
            else if (errno == E2BIG) {
                handle_e2big(output, outbuf, outbytes);
            }
            else {
                throw std::system_error(errno, std::system_category(), "iconv");
            }
        }
        else {
            break;
        }
    }

    if (result != static_cast<std::size_t>(-1)) {
        handle_tail(cd, output, outbuf, outbytes);
    }

    return {output.data(), output.size() - outbytes};
}

void converter_iconv::free_iconv(iconv_t cd)
{
    if (cd != nullptr && intptr_cast(cd) != -1) {
        iconv_close(cd);
    }
}
