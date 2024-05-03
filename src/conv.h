#ifndef AC12FED9_D844_4F7B_A90A_9CA8C2D27A08
#define AC12FED9_D844_4F7B_A90A_9CA8C2D27A08

#include <memory>
#include <string>
#include <iconv.h>

class converter_iconv {
public:
    converter_iconv();
    void set_charsets(const std::string& from, const std::string& to);
    std::string operator()(const std::string& str) const;

private:
    static void free_iconv(iconv_t cd);
    mutable std::unique_ptr<std::remove_pointer_t<iconv_t>, decltype(&converter_iconv::free_iconv)> m_cd;
    bool m_ignore = false;
};

#endif /* AC12FED9_D844_4F7B_A90A_9CA8C2D27A08 */
