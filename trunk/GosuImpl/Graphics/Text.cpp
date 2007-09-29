#include <Gosu/Text.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Math.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <vector>
using namespace std;

namespace Gosu
{
    namespace
    {
        struct WordInfo
        {
            wstring text;
            unsigned width;
        };
        typedef vector<WordInfo> Words;

        // Local helper class which manages building the bitmap from the
        // collected words.
        class TextBlockBuilder
        {
            Bitmap bmp;
            unsigned usedLines, allocatedLines;

            wstring fontName;
            unsigned fontHeight, fontFlags, lineSpacing;
            TextAlign align;

            unsigned spaceWidth;

            void allocNextLine()
            {
                ++usedLines;
                if (usedLines == allocatedLines)
                {
                    allocatedLines += 10;
                    bmp.resize(bmp.width(),
                        (lineSpacing + fontHeight) * allocatedLines,
                        Colors::none);
                }
            }

        public:
            TextBlockBuilder(const wstring& fontName, unsigned fontHeight,
                unsigned fontFlags, unsigned lineSpacing, unsigned width,
                TextAlign align)
            {
                usedLines = 0;
                allocatedLines = 10;
                
                bmp.resize(width, (lineSpacing + fontHeight) * allocatedLines, 0x00ffffff);

                this->fontName = fontName;
                this->fontHeight = fontHeight;
                this->fontFlags = fontFlags;
                this->lineSpacing = lineSpacing;
                this->align = align;

                spaceWidth = textWidth(L" ");
            }

            unsigned width() const
            {
                return bmp.width();
            }

            unsigned textWidth(const std::wstring& text) const
            {
                return Gosu::textWidth(text, fontName, fontHeight, fontFlags);
            }

            void addLine(Words::const_iterator begin, Words::const_iterator end,
                unsigned wordsWidth, bool overrideAlign)
            {
                allocNextLine();
                
                unsigned words = end - begin;

                // Where does the line start? (y)
                unsigned top = (usedLines - 1) * (fontHeight + lineSpacing);

                // Where does the line start? (x)
                double pos;
                switch (align)
                {
                // Start so that the text touches the right border.
                case taRight:
                    pos = bmp.width() - wordsWidth - (words - 1) * spaceWidth;
                    break;

                // Start so that the text is centered.
                case taCenter:
                    pos = bmp.width() - wordsWidth - (words - 1) * spaceWidth;
                    pos /= 2;
                    break;

                // Just start at the left border.
                default:
                    pos = 0;
                }

                // How much space is between each word?
                double spacing;
                if (align == taJustify && !overrideAlign)
                    spacing = (bmp.width() - wordsWidth) / (words - 1.0);
                else
                    spacing = spaceWidth;

                for (Words::const_iterator cur = begin; cur != end; ++cur)
                {
                    drawText(bmp, cur->text, trunc(pos), trunc(top),
                        Colors::white, fontName, fontHeight, fontFlags);
                    pos += cur->width + spacing;
                }
            }

            Bitmap result() const
            {
                Bitmap result = bmp;
                result.resize(result.width(),
                    usedLines * (lineSpacing + fontHeight));
                return result;
            }
        };

        void processWords(TextBlockBuilder& builder, const Words& words)
        {
            // Index into words to the first word in the current line.
            Words::const_iterator lineBegin = words.begin();

            const unsigned spaceWidth = builder.textWidth(L" "); // IMPR.

            // Used width, in pixels, of the words [lineBegin..w[.
            unsigned wordsWidth = 0;

            // Used width of the spaces between (w-lineBegin) words.
            unsigned spacesWidth = 0;

            for (Words::const_iterator w = words.begin(); w != words.end(); ++w)
            {
                unsigned newWordsWidth = wordsWidth + w->width;

                if (newWordsWidth + spacesWidth <= builder.width())
                {
                    // There's enough space for the words [lineBegin..w] plus
                    // the spaces between them: Proceed with the next word.
                    wordsWidth = newWordsWidth;
                    spacesWidth += spaceWidth;
                }
                else
                {
                    // No, this word wouldn't fit into the current line: Draw
                    // the current line, then start a new line with the current
                    // word.
                    builder.addLine(lineBegin, w, wordsWidth, false);

                    lineBegin = w;
                    wordsWidth = w->width;
                    spacesWidth = spaceWidth;
                }
            }

            // Draw the last line as well.
            if (words.empty() || lineBegin != words.end())
                builder.addLine(lineBegin, words.end(), wordsWidth, true);
        }

        typedef boost::tokenizer<boost::char_separator<wchar_t>,
            wstring::const_iterator, wstring> Tokenizer;

        void processParagraph(TextBlockBuilder& builder,
            const wstring& paragraph)
        {
            Words collectedWords;

            boost::char_separator<wchar_t> sep(L" ");
            Tokenizer words(paragraph, sep);
            for (Tokenizer::iterator i = words.begin(); i != words.end(); ++i)
            {
                WordInfo newWord;
                newWord.text = *i;
                newWord.width = builder.textWidth(newWord.text);
                collectedWords.push_back(newWord);
            }

            processWords(builder, collectedWords);
        }

        void processText(TextBlockBuilder& builder, const wstring& text)
        {
            vector<wstring> paragraphs;
            wstring processedText = boost::replace_all_copy(text, L"\r\n", L"\n");
            boost::split(paragraphs, processedText,
                boost::is_any_of(L"\r\n"));
            for_each(paragraphs.begin(), paragraphs.end(),
                boost::bind(processParagraph, boost::ref(builder), _1));
        }
    }
}

Gosu::Bitmap Gosu::createText(const std::wstring& text,
    const std::wstring& fontName, unsigned fontHeight, unsigned lineSpacing,
    unsigned maxWidth, TextAlign align, unsigned fontFlags)
{
    // Set up the builder object which will manage all the drawing and
    // conversions for us.
    TextBlockBuilder builder(fontName, fontHeight, fontFlags,
        lineSpacing, maxWidth, align);

    // Let the process* functions draw everything.
    processText(builder, text);

    // Done!
    return builder.result();
}
