#include <Gosu/Text.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/Utility.hpp>
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
        // Just a very simple heuristic that should make Chinese and Japanese text working in most
        // of the cases.
        bool isBreakingAsianGlyph(wchar_t ch)
        {
            if (ch >= 0x3040 && ch <= 0x3096)
                return true; // Hiragana
            
            if (ch >= 0x30a0 && ch <= 0x30fa)
                return true; // Katakana
            
            if (ch >= 0x4e00 && ch <= 0x9fff)
                return true; // CJK Unified Ideographs
                
            if (ch >= 0x3400 && ch <= 0x4db5)
                return true; // CJK Unified Ideographs Extension A
            
            return false;
        }
    
        struct WordInfo
        {
            wstring text;
            unsigned width;
            unsigned spaceWidth;
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
                        Color::NONE);
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
                
                unsigned totalSpacing = 0;
                for (Words::const_iterator i = begin; i != end - 1; ++i)
                    totalSpacing += i->spaceWidth;

                // Where does the line start? (y)
                unsigned top = (usedLines - 1) * (fontHeight + lineSpacing);

                // Where does the line start? (x)
                double pos;
                switch (align)
                {
                // Start so that the text touches the right border.
                case taRight:
                    pos = bmp.width() - wordsWidth - totalSpacing;
                    break;

                // Start so that the text is centered.
                case taCenter:
                    pos = bmp.width() - wordsWidth - totalSpacing;
                    pos /= 2;
                    break;

                // Just start at the left border.
                default:
                    pos = 0;
                }

                for (Words::const_iterator cur = begin; cur != end; ++cur)
                {
                    drawText(bmp, cur->text, trunc(pos), trunc(top),
                        Color::WHITE, fontName, fontHeight, fontFlags);

                    if (align == taJustify && !overrideAlign)
                        pos += cur->width + 1.0 * (width() - wordsWidth) / (words - 1);
                    else
                        pos += cur->width + cur->spaceWidth;
                }
            }
            
            void addEmptyLine()
            {
                allocNextLine();
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
            if (words.empty())
                return builder.addEmptyLine();

            // Index into words to the first word in the current line.
            Words::const_iterator lineBegin = words.begin();

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
                    spacesWidth += w->spaceWidth;
                }
                else
                {
                    // No, this word wouldn't fit into the current line: Draw
                    // the current line, then start a new line with the current
                    // word.
                    builder.addLine(lineBegin, w, wordsWidth, false);

                    lineBegin = w;
                    wordsWidth = w->width;
                    spacesWidth = w->spaceWidth;
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

            const unsigned spaceWidth = builder.textWidth(L" ");

            unsigned beginOfWord = 0;
            
            for (unsigned cur = 0; cur < paragraph.length(); ++cur)
            {
                WordInfo newWord;

                if (paragraph[cur] == L' ')
                {
                    // Whitespace: Add last word to list if existent
                    if (beginOfWord != cur)
                    {
                        newWord.text.assign(paragraph.begin() + beginOfWord, paragraph.begin() + cur);
                        newWord.width = builder.textWidth(newWord.text);
                        newWord.spaceWidth = spaceWidth;
                        collectedWords.push_back(newWord);
                    }
                    beginOfWord = cur + 1;
                }
                else if (isBreakingAsianGlyph(paragraph[cur]))
                {
                    // Whitespace: Add last word to list if existent
                    if (beginOfWord != cur)
                    {
                        newWord.text.assign(paragraph.begin() + beginOfWord, paragraph.begin() + cur);
                        newWord.width = builder.textWidth(newWord.text);
                        newWord.spaceWidth = 0;
                        collectedWords.push_back(newWord);
                    }
                    // Add glyph as a single "word"
                    newWord.text = wstring(1, paragraph[cur]);
                    newWord.width = builder.textWidth(newWord.text);
                    newWord.spaceWidth = 0;
                    collectedWords.push_back(newWord);
                    beginOfWord = cur + 1;
                }
            }
            if (beginOfWord < paragraph.length())
            {
                WordInfo lastWord;
                lastWord.text.assign(paragraph.begin() + beginOfWord, paragraph.end());
                lastWord.width = builder.textWidth(lastWord.text);
                lastWord.spaceWidth = 0;
                collectedWords.push_back(lastWord);
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
    if (text.empty())
    {
        Bitmap emptyBitmap;
        emptyBitmap.resize(1, fontHeight);
        return emptyBitmap;
    }
    
    // Set up the builder object which will manage all the drawing and
    // conversions for us.
    TextBlockBuilder builder(fontName, fontHeight, fontFlags,
        lineSpacing, maxWidth, align);
    
    // Let the process* functions draw everything.
    processText(builder, text);
    
    // Done!
    return builder.result();
}

// Very easy special case.
Gosu::Bitmap Gosu::createText(const std::wstring& text,
    const std::wstring& fontName, unsigned fontHeight, unsigned fontFlags)
{
    if (text.empty())
    {
        Bitmap emptyBitmap;
        emptyBitmap.resize(1, fontHeight);
        return emptyBitmap;
    }

    vector<wstring> lines;
    wstring processedText = boost::replace_all_copy(text, L"\r\n", L"\n");
    boost::split(lines, processedText, boost::is_any_of(L"\r\n"));

    Bitmap bmp;
    bmp.resize(textWidth(lines.front(), fontName, fontHeight, fontFlags), fontHeight);
    drawText(bmp, text, 0, 0, 0xffffffff, fontName, fontHeight, fontFlags);
    
    for (int i = 1; i < lines.size(); ++i)
    {
        bmp.resize(max(bmp.width(), textWidth(lines[i], fontName, fontHeight, fontFlags)),
                   bmp.height() + fontHeight);
        drawText(bmp, lines[i], 0, fontHeight * i, 0xffffffff, fontName, fontHeight, fontFlags);
    }
    
    return bmp;
}
