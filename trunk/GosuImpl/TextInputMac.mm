#include <Gosu/TextInput.hpp>
#include <Gosu/ButtonsMac.hpp>
#include <GosuImpl/MacUtility.hpp>
#include <algorithm>
#include <vector>
#include <wctype.h>

#import <Cocoa/Cocoa.h>

// Sometimes macro DSLs aren't evil.
#define CARET_POS (*_caretPos)
#define SEL_START (*_selectionStart)
#define IMPL_WITH_SEL_RESET(selector)           \
- (void)selector:(id)sender                     \
{                                               \
    [self selector##AndModifySelection:sender]; \
    SEL_START = CARET_POS;                      \
}

@interface GosuTextInput : NSResponder
{
    Gosu::TextInput* _filter;
    std::wstring* _text;
    unsigned* _caretPos;
    unsigned* _selectionStart;
}
@end
@implementation GosuTextInput
- (void)insertText:(id)insertString
{
    // Delete (overwrite) previous selection.
    if (CARET_POS != SEL_START)
    {
        unsigned min = std::min(CARET_POS, SEL_START);
        unsigned max = std::max(CARET_POS, SEL_START);
        _text->erase(_text->begin() + min, _text->begin() + max);
        CARET_POS = SEL_START = min;
    }
    
    NSString* str = (NSString*)insertString;
    
    if ([str length] == 0)
        return;
    
    std::vector<unichar> unibuf([str length]);
    [str getCharacters: &unibuf[0]];
    
    std::wstring text = _filter->filter(std::wstring(unibuf.begin(), unibuf.end()));
    
    _text->insert(_text->begin() + CARET_POS, text.begin(), text.end());
    (CARET_POS) += text.size();
    (SEL_START) = CARET_POS;
}

// Movement.

- (void)moveLeftAndModifySelection:(id)sender
{
    if (CARET_POS > 0)
        (CARET_POS) -= 1;
}
- (void)moveRightAndModifySelection:(id)sender
{
    if (CARET_POS < _text->length())
        (CARET_POS) += 1;
}
- (void)moveUpAndModifySelection:(id)sender
{
    CARET_POS = 0;
}
- (void)moveDownAndModifySelection:(id)sender
{
    CARET_POS = _text->length();
}
- (void)moveToBeginningOfLineAndModifySelection:(id)sender
{
    [self moveUpAndModifySelection: sender];
}
- (void)moveToEndOfLineAndModifySelection:(id)sender
{
    [self moveDownAndModifySelection: sender];
}
- (void)moveToBeginningOfDocumentAndModifySelection:(id)sender
{
    [self moveUpAndModifySelection: sender];
}
- (void)moveToEndOfDocumentAndModifySelection:(id)sender
{
    [self moveDownAndModifySelection: sender];
}
- (void)moveWordLeftAndModifySelection:(id)sender
{
    if (CARET_POS == _text->length())
        --CARET_POS;
    
    while (CARET_POS > 0 && iswspace(_text->at(CARET_POS - 1)))
        --CARET_POS;

    while (CARET_POS > 0 && !iswspace(_text->at(CARET_POS - 1)))
        --CARET_POS;
}
- (void)moveWordRightAndModifySelection:(id)sender
{
    while (CARET_POS < _text->length() && iswspace(_text->at(CARET_POS)))
        ++CARET_POS;

    while (CARET_POS < _text->length() && !iswspace(_text->at(CARET_POS)))
        ++CARET_POS;
}

IMPL_WITH_SEL_RESET(moveLeft)
IMPL_WITH_SEL_RESET(moveRight)
IMPL_WITH_SEL_RESET(moveUp)
IMPL_WITH_SEL_RESET(moveDown)
IMPL_WITH_SEL_RESET(moveWordLeft)
IMPL_WITH_SEL_RESET(moveWordRight)
IMPL_WITH_SEL_RESET(moveToBeginningOfLine)
IMPL_WITH_SEL_RESET(moveToEndOfLine)
IMPL_WITH_SEL_RESET(moveToBeginningOfDocument)
IMPL_WITH_SEL_RESET(moveToEndOfDocument)

// Deletion.

- (void)deleteBackward:(id)sender
{
    if (CARET_POS == SEL_START && CARET_POS > 0)
        --CARET_POS;
    [self insertText: @""];
}
- (void)deleteForward:(id)sender
{
    if (CARET_POS == SEL_START && CARET_POS < _text->length())
        ++CARET_POS;
    [self insertText: @""];
}

// TODO: Advanced deletion (deleteWord...)

// Selection.

- (void)selectAll:(id)sender
{
    SEL_START = 0;
    CARET_POS = _text->length();
}

// TODO: Cmd+A should really be hard-coded into selectAll.

// Helper to set up this instance.

- (void)setFilter:(Gosu::TextInput&)filter andText:(std::wstring&)text
    andCaretPos:(unsigned&)caretPos andSelectionStart:(unsigned&)selectionStart
{
    _filter = &filter;
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
    [pimpl->responder.get() setFilter: *this andText: pimpl->text
        andCaretPos: pimpl->caretPos andSelectionStart: pimpl->selectionStart];
}

Gosu::TextInput::~TextInput()
{
}

std::wstring Gosu::TextInput::text() const
{
    return pimpl->text;
}

void Gosu::TextInput::setText(const std::wstring& text)
{
	pimpl->text = text;
	pimpl->caretPos = pimpl->selectionStart = text.length();
}

unsigned Gosu::TextInput::caretPos() const
{
    return pimpl->caretPos;
}

unsigned Gosu::TextInput::selectionStart() const
{
    return pimpl->selectionStart;
}

bool Gosu::TextInput::feedNSEvent(void* event)
{
	NSEvent* nsEvent = (NSEvent*)event;
	long keyCode = [nsEvent keyCode];
	if (keyCode != kbEscape && keyCode != kbTab && keyCode != kbReturn && keyCode != kbEnter)
	{
		[pimpl->responder.get() interpretKeyEvents: [NSArray arrayWithObject: nsEvent]];
		return true;
	}
	else
		return false;
}
