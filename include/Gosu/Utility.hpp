#pragma once

#include <string>
#include <vector>

namespace Gosu
{
    /// Converts a UTF-8 string to composed UTC-4, i.e. to a list of Unicode codepoints.
    ///
    /// This is not quite as good as splitting a string into graphemes. For example, the "family of
    /// four" Emoji is actually composed of seven(!) codepoints. But because Gosu's text rendering
    /// facilities have no support for multi-codepoint graphemes, Gosu::Font and related classes are
    /// based on codepoints.
    ///
    /// This method will silently skip over invalid UTF-8 sequences.
    std::u32string utf8_to_composed_utc4(const std::string& utf8);

    /// Returns true if the filename has the given extension.
    /// The comparison is case-insensitive, but you must pass the extension in lower case.
    /// If the extension does not start with a dot, then it will be implied.
    bool has_extension(std::string_view filename, std::string_view extension);

    /// Returns the user's preferred user_languages/locales, e.g. {"en_US", "de_DE", "ja"}.
    /// The first two letters of each element will always be a language code.
    /// This value is not cached. Please memorize the specific value that you are interested in,
    /// typically the first language in the returned array that your game supports.
    std::vector<std::string> user_languages();

    /// Base class to quickly make a type non-copyable/non-movable. Same as boost::noncopyable.
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

    /// An int-based rectangle class, mostly for internal use.
    struct Rect
    {
        int x = 0, y = 0;
        int width = 0, height = 0;

        int right() const { return x + width; }
        int bottom() const { return y + height; }

        bool empty() const { return width <= 0 || height <= 0; }

        /// Returns a Rect that starts at the origin and uses T::width() and T::height() as its
        /// extents.
        template <typename T> static Rect covering(const T& object)
        {
            return Rect { .x = 0, .y = 0, .width = object.width(), .height = object.height() };
        }

        bool overlaps(const Rect& other) const;
        bool contains(const Rect& other) const;

        /// Makes sure that this rectangle does not exceed the bounding box.
        /// @param adjust_x If the rectangle origin will be moved to the right by this operation,
        ///                 then the pointed-to int (if any) will be adjusted by the same amount.
        /// @param adjust_y If the rectangle origin will be moved down by this operation,
        ///                 then the pointed-to int (if any) will be adjusted by the same amount.
        void clip_to(const Rect& bounding_box, int* adjust_x = nullptr, int* adjust_y = nullptr);

        bool operator==(const Rect& other) const = default;
    };
}
