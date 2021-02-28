#pragma once

#include <string>
#include <vector>

namespace Gosu
{
    std::u32string utf8_to_composed_utc4(const std::string& utf8);

    /// Returns true if the filename has the given extension.
    /// The comparison is case-insensitive, but you must pass the extension in lower case.
    bool has_extension(std::string_view filename, std::string_view extension);

    /// Returns the user's preferred user_languages/locales, e.g. {"en_US", "de_DE", "ja"}.
    /// The first two letters of each element will always be a language code.
    /// This value is not cached. Please memorize the specific value that you are interested in,
    /// typically the first language in the returned array that your game supports.
    std::vector<std::string> user_languages();

    class Noncopyable
    {
    protected:
        Noncopyable() = default;
        ~Noncopyable() = default;

    public:
        Noncopyable(const Noncopyable& other) = delete;
        Noncopyable& operator=(const Noncopyable& other) = delete;

        Noncopyable(Noncopyable&& other) = delete;
        Noncopyable& operator=(Noncopyable&& other) = delete;
    };
}
