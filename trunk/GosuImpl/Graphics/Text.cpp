#include <Gosu/Text.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/Utility.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <GosuImpl/Graphics/FormattedString.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <map>
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
            FormattedString text;
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
            unsigned fontHeight, lineSpacing;
            TextAlign align;

            unsigned spaceWidth_;

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
                unsigned lineSpacing, unsigned width, TextAlign align)
            {
                usedLines = 0;
                allocatedLines = 10;
                
                bmp.resize(width, (lineSpacing + fontHeight) * allocatedLines, 0x00ffffff);

                this->fontName = fontName;
                this->fontHeight = fontHeight;
                this->lineSpacing = lineSpacing;
                this->align = align;

                spaceWidth_ = textWidth(FormattedString(L" ", 0));
            }

            unsigned width() const
            {
                return bmp.width();
            }

            unsigned textWidth(const FormattedString& text) const
            {
                if (text.length() == 0)
                    return 0;
                
                if (text.entityAt(0))
                    return entityBitmap(text.entityAt(0)).width();
                
                std::vector<FormattedString> parts = text.splitParts();
                unsigned result = 0;
                BOOST_FOREACH (const FormattedString& part, parts)
                    result += Gosu::textWidth(part.unformat(), fontName, fontHeight, part.flagsAt(0));
                return result;
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
                    std::vector<FormattedString> parts = cur->text.splitParts();
                    int x = 0;
                    BOOST_FOREACH (const FormattedString& part, parts)
                    {
                        if (part.entityAt(0))
                        {
                            Gosu::Bitmap entity = entityBitmap(part.entityAt(0));
                            multiplyBitmapAlpha(entity, part.colorAt(0).alpha());
                            bmp.insert(entity, trunc(pos) + x, trunc(top));
                            x += entity.width();
                            continue;
                        }
                        
                        std::wstring unformattedPart = part.unformat();
                        drawText(bmp, unformattedPart, trunc(pos) + x, trunc(top),
                            part.colorAt(0), fontName, fontHeight, part.flagsAt(0));
                        x += Gosu::textWidth(unformattedPart, fontName, fontHeight,
                            part.flagsAt(0));
                    }
                    
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
            
            unsigned spaceWidth() const
            {
                return spaceWidth_;
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
        
        void processParagraph(TextBlockBuilder& builder, const FormattedString& paragraph)
        {
            Words collectedWords;
            
            unsigned beginOfWord = 0;
            
            for (unsigned cur = 0; cur < paragraph.length(); ++cur)
            {
                WordInfo newWord;

                if (paragraph.charAt(cur) == L' ')
                {
                    // Whitespace:
                    // Add last word to list if existent
                    if (beginOfWord != cur)
                    {
                        newWord.text = paragraph.range(beginOfWord, cur);
                        newWord.width = builder.textWidth(newWord.text);
                        newWord.spaceWidth = builder.spaceWidth();
                        collectedWords.push_back(newWord);
                    }
                    beginOfWord = cur + 1;
                }
                else if (isBreakingAsianGlyph(paragraph.charAt(cur)))
                {
                    // Asian glyph (treat as single word):
                    // Add last word to list if existent
                    if (beginOfWord != cur)
                    {
                        newWord.text = paragraph.range(beginOfWord, cur);
                        newWord.width = builder.textWidth(newWord.text);
                        newWord.spaceWidth = 0;
                        collectedWords.push_back(newWord);
                    }
                    // Add glyph as a single "word"
                    newWord.text = paragraph.range(cur, cur + 1);
                    newWord.width = builder.textWidth(newWord.text);
                    newWord.spaceWidth = 0;
                    collectedWords.push_back(newWord);
                    beginOfWord = cur + 1;
                }
            }
            if (beginOfWord < paragraph.length())
            {
                WordInfo lastWord;
                lastWord.text = paragraph.range(beginOfWord, paragraph.length());
                lastWord.width = builder.textWidth(lastWord.text);
                lastWord.spaceWidth = 0;
                collectedWords.push_back(lastWord);
            }
            
            processWords(builder, collectedWords);
        }

        void processText(TextBlockBuilder& builder, const FormattedString& text)
        {
            vector<FormattedString> paragraphs = text.splitLines();
            BOOST_FOREACH (FormattedString& fs, paragraphs)
                processParagraph(builder, fs);
        }
    }
}

Gosu::Bitmap Gosu::createText(const std::wstring& text,
    const std::wstring& fontName, unsigned fontHeight, unsigned lineSpacing,
    unsigned maxWidth, TextAlign align, unsigned fontFlags)
{
    FormattedString fs(boost::replace_all_copy(text, L"\r\n", L"\n"), fontFlags);
    if (fs.length() == 0)
    {
        Bitmap emptyBitmap;
        emptyBitmap.resize(1, fontHeight);
        return emptyBitmap;
    }
    
    // Set up the builder object which will manage all the drawing and
    // conversions for us.
    TextBlockBuilder builder(fontName, fontHeight,
        lineSpacing, maxWidth, align);
    
    // Let the process* functions draw everything.
    processText(builder, fs);
    
    // Done!
    return builder.result();
}

// Very easy special case.
Gosu::Bitmap Gosu::createText(const std::wstring& text,
    const std::wstring& fontName, unsigned fontHeight, unsigned fontFlags)
{
    Bitmap bmp;
    bmp.resize(1, fontHeight);
    
    FormattedString fs(boost::replace_all_copy(text, L"\r\n", L"\n"), fontFlags);
    if (fs.length() == 0)
        return bmp;
    
    vector<FormattedString> lines = fs.splitLines();

    for (int i = 0; i < lines.size(); ++i)
    {
        bmp.resize(bmp.width(), (i + 1) * fontHeight);

        if (lines[i].length() == 0)
            continue;
        
        unsigned x = 0;
        std::vector<FormattedString> parts = lines[i].splitParts();
        for (int p = 0; p < parts.size(); ++p)
        {
            const FormattedString& part = parts[p];
            if (part.length() == 1 && part.entityAt(0))
            {
                Gosu::Bitmap entity = entityBitmap(part.entityAt(0));
                multiplyBitmapAlpha(entity, part.colorAt(0).alpha());
                bmp.resize(std::max(bmp.width(), x + entity.width()), bmp.height());
                bmp.insert(entity, x, i * fontHeight);
                x += entity.width();
                continue;
            }
                
            assert(part.length() > 0);
            std::wstring unformattedText = part.unformat();
            unsigned partWidth =
                textWidth(unformattedText, fontName, fontHeight, part.flagsAt(0));
            bmp.resize(std::max(bmp.width(), x + partWidth), bmp.height());
            drawText(bmp, unformattedText, x, i * fontHeight, part.colorAt(0),
                fontName, fontHeight, part.flagsAt(0));
            x += partWidth;
        }
    }
    
    return bmp;
}

namespace
{
    std::map<std::wstring, boost::shared_ptr<Gosu::Bitmap> > entities;
}

void Gosu::registerEntity(const std::wstring& name, const Gosu::Bitmap& replacement)
{
    entities[name].reset(new Bitmap(replacement));
}

const Gosu::Bitmap& Gosu::entityBitmap(const std::wstring& name)
{
    return *entities[name];
}
