#include <Gosu/Utility.hpp>
#include <Gosu/Platform.hpp>

#include <utf8proc.h>

#include <cstring>
#include <stdexcept>
using namespace std;

u32string Gosu::utf8_to_composed_utc4(const string& utf8)
{
    u32string utc4;
    utc4.reserve(utf8.size());
    
    auto* current_byte = reinterpret_cast<const utf8proc_uint8_t*>(utf8.data());
    auto remaining_length = utf8.length();
    
    // First convert from UTF-8 to UTC-4.
    utf8proc_int32_t codepoint;
    while (remaining_length) {
        auto bytes_read = utf8proc_iterate(current_byte, remaining_length, &codepoint);
        if (bytes_read == -1) {
            // Not looking good, skip this byte and retry.
            current_byte += 1;
            remaining_length -= 1;
        }
        else {
            utc4.push_back(codepoint);
            current_byte += bytes_read;
            remaining_length -= bytes_read;
        }
    }
    
    // Now compose characters in-place.
    auto utc4_data = reinterpret_cast<const utf8proc_int32_t*>(utc4.data());
    // Note that this const_cast will not be necessary anymore in C++17.
    auto writable_utc4_data = const_cast<utf8proc_int32_t*>(utc4_data);
    auto options = static_cast<utf8proc_option_t>(UTF8PROC_NLF2LF | UTF8PROC_COMPOSE);
    auto new_length = utf8proc_normalize_utf32(writable_utc4_data, utc4.length(), options);
    if (new_length < 0) {
        throw runtime_error("Could not normalize '" + utf8 + "': " + utf8proc_errmsg(new_length));
    }
    utc4.resize(new_length);
    
    return utc4;
}

bool Gosu::has_extension(string_view filename, string_view extension)
{
    size_t ext_len = extension.length();
    if (ext_len > filename.length()) {
        return false;
    }

    string_view::iterator filename_iter = filename.end();
    string_view::iterator ext_iter = extension.end();
    while (ext_len--) {
        --filename_iter;
        --ext_iter;

        if (tolower((int) *filename_iter) != tolower((int) *ext_iter)) {
            return false;
        }
    }

    return true;
}

#if defined(GOSU_IS_UNIX) && !defined(GOSU_IS_MAC)
string Gosu::language()
{
    const char* env = getenv("LANG");
    return env ? env : "en_US";
}
#endif
