#include <Gosu/TextInput.hpp>
#include <Gosu/MacUtility.hpp>
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
    NSString* str = (NSString*)insertString;
    std::vector<unichar> unibuf([str length]);
    [str getCharacters: &unibuf[0]];
    
    _text->insert(_text->begin() + *_caretPos, unibuf.begin(), unibuf.end());
    (*_caretPos) += unibuf.size();
    (*_selectionStart) = *_caretPos;
}
- (void)moveLeft:(id)sender
{
    if (*_caretPos > 0)
        (*_caretPos) -= 1;
    (*_selectionStart) = *_caretPos;
}
- (void)moveRight:(id)sender
{
    if (*_caretPos < _text->length())
        (*_caretPos) += 1;
    (*_selectionStart) = *_caretPos;
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
