#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>

#include <utf8proc.h>

#include <cstring>
#include <memory>
#include <stdexcept>

std::u32string Gosu::utf8_to_composed_utc4(const std::string& utf8)
{
    std::u32string utc4;
    utc4.reserve(utf8.size());

    const auto* current_byte = reinterpret_cast<const utf8proc_uint8_t*>(utf8.data());
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
    auto* utc4_data = reinterpret_cast<utf8proc_int32_t*>(utc4.data());
    auto options = static_cast<utf8proc_option_t>(UTF8PROC_NLF2LF | UTF8PROC_COMPOSE);
    auto new_length = utf8proc_normalize_utf32(utc4_data, utc4.length(), options);
    if (new_length < 0) {
        throw std::runtime_error{"Could not normalize '" + utf8 +
                                 "': " + utf8proc_errmsg(new_length)};
    }
    utc4.resize(new_length);

    return utc4;
}

bool Gosu::has_extension(std::string_view filename, std::string_view extension)
{
    std::size_t ext_len = extension.length();
    if (ext_len > filename.length()) {
        return false;
    }

    std::string_view::iterator filename_iter = filename.end();
    std::string_view::iterator ext_iter = extension.end();
    while (ext_len--) {
        --filename_iter;
        --ext_iter;

        if (std::tolower(static_cast<int>(*filename_iter)) !=
            std::tolower(static_cast<int>(*ext_iter))) {
            return false;
        }
    }

    return true;
}

#ifdef GOSU_IS_IPHONE
#import <Foundation/Foundation.h>
#include <regex>

std::vector<std::string> Gosu::user_languages()
{
    static const std::regex language_regex{"([a-z]{2})-([A-Z]{2})([^A-Z].*)?"};

    std::vector<std::string> user_languages;

    @autoreleasepool {
        for (NSString* language in [NSLocale preferredLanguages]) {
            std::string language_str = language.UTF8String;
            std::smatch match;
            if (std::regex_match(language_str, match, language_regex)) {
                user_languages.push_back(match.str(1) + "_" + match.str(2));
            }
        }
    }

    return user_languages;
}

#else
#include <SDL.h>
#if SDL_VERSION_ATLEAST(2, 0, 14)

std::vector<std::string> Gosu::user_languages()
{
    std::vector<std::string> user_languages;

    std::unique_ptr<SDL_Locale, decltype(SDL_free)*> locales{SDL_GetPreferredLocales(), SDL_free};
    if (!locales) return user_languages;

    for (const SDL_Locale* locale = locales.get(); locale->language != nullptr; ++locale) {
        std::string language = locale->language;
        if (locale->country) {
            language += "_";
            language += locale->country;
        }
        user_languages.emplace_back(std::move(language));
    }

    return user_languages;
}

#else
#include <cstdlib>
#include <regex>

std::vector<std::string> Gosu::user_languages()
{
    static const std::regex language_regex{"[a-z]{2}_[A-Z]{2}([^A-Z].*)?"};

    const char* locale = std::getenv("LANG");

    if (locale && std::regex_match(locale, language_regex)) {
        // Trim off anything after the language code.
        return {std::string{locale, locale + 5}};
    }

    return {};
}

#endif
#endif
