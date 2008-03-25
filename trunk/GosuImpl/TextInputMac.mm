#include <Gosu/TextInput.hpp>
#include <Gosu/MacUtility.hpp>
#include <algorithm>
#include <vector>

#import <Cocoa/Cocoa.h>

@interface GosuTextInput : NSResponder
{
    std::wstring* _text;
    unsigned* _caretPos;
    unsigned* _selectionStart;
}
@end
@implementation GosuTextInput
- (void)insertText:(id)insertString
{
    // Delete (overwrite) previous selection.
    if (*_caretPos != *_selectionStart)
    {
        unsigned min = std::min(*_caretPos, *_selectionStart);
        unsigned max = std::max(*_caretPos, *_selectionStart);
        _text->erase(_text->begin() + min, _text->begin() + max);
        *_caretPos = *_selectionStart = min;
    }
    
    NSString* str = (NSString*)insertString;
    
    if ([str length] == 0)
        return;
    
    std::vector<unichar> unibuf([str length]);
    [str getCharacters: &unibuf[0]];
    
    _text->insert(_text->begin() + *_caretPos, unibuf.begin(), unibuf.end());
    (*_caretPos) += unibuf.size();
    (*_selectionStart) = *_caretPos;

    std::wstring(*_text).length();
}
- (void)moveLeft:(id)sender
{
    if (*_caretPos > 0)
        (*_caretPos) -= 1;
    (*_selectionStart) = *_caretPos;

    std::wstring(*_text).length();
}
- (void)moveRight:(id)sender
{
    if (*_caretPos < _text->length())
        (*_caretPos) += 1;
    (*_selectionStart) = *_caretPos;

    std::wstring(*_text).length();
}
- (void)moveLeftAndModifySelection:(id)sender
{
    if (*_caretPos > 0)
        (*_caretPos) -= 1;

    std::wstring(*_text).length();
}
- (void)moveRightAndModifySelection:(id)sender
{
    if (*_caretPos < _text->length())
        (*_caretPos) += 1;

    std::wstring(*_text).length();
}
- (void)moveToBeginningOfLine:(id)sender
{
    *_caretPos = *_selectionStart = 0;

    std::wstring(*_text).length();
}
- (void)moveToEndOfLine:(id)sender
{
    *_caretPos = *_selectionStart = _text->length();

    std::wstring(*_text).length();
}
- (void)deleteBackward:(id)sender
{
    if (*_caretPos == *_selectionStart && *_caretPos > 0)
        --*_caretPos;
    [self insertText: @""];

    std::wstring(*_text).length();
}
- (void)deleteForward:(id)sender
{
    if (*_caretPos == *_selectionStart && *_caretPos < _text->length())
        ++*_caretPos;
    [self insertText: @""];

    std::wstring(*_text).length();
}

- (void)setText:(std::wstring&)text andCaretPos:(unsigned&)caretPos andSelectionStart:(unsigned&)selectionStart
{
    _text = &text;
    _caretPos = &caretPos;
    _selectionStart = &selectionStart;
}
@end

struct Gosu::TextInput::Impl
{
    ObjRef<GosuTextInput> responder;
    std::wstring text;
    unsigned caretPos, selectionStart;
    Impl() : caretPos(0), selectionStart(0) {}
};

Gosu::TextInput::TextInput()
: pimpl(new Impl)
{
    pimpl->responder.reset([[GosuTextInput alloc] init]);
    [pimpl->responder.get() setText: pimpl->text andCaretPos: pimpl->caretPos andSelectionStart: pimpl->selectionStart];
}

Gosu::TextInput::~TextInput()
{
}

std::wstring Gosu::TextInput::text() const
{
    return pimpl->text;
}

unsigned Gosu::TextInput::caretPos() const
{
    return pimpl->caretPos;
}

unsigned Gosu::TextInput::selectionStart() const
{
    return pimpl->selectionStart;
}

void Gosu::TextInput::feedNSEvent(void* event)
{
    [pimpl->responder.get() interpretKeyEvents: [NSArray arrayWithObject: (NSEvent*)event]];
}
