// This example demonstrates the use of the TextInput functionality.
// One can tab through, or click into the text fields and change it's contents.

// At its most basic form, you only need to create a new TextInput instance and
// pass it to your window via setTextInput. Until you call this function again,
// passing 0, the TextInput object will build a text that can be accessed via
// TextInput::text().

// The TextInput object also maintains the position of the caret as the index
// of the character that it's left to via the caretPos() member function.
// Furthermore, if there is a selection, the selectionStart() member yields its
// beginning, using the same indexing scheme. If there is no selection,
// selectionStart() is equal to caretPos().

// A TextInput object is purely abstract, though; drawing the input field is left
// to the user. In this case, we are subclassing TextInput to add this code.
// As with most of Gosu, how this is handled is completely left open; the scheme
// presented here is not mandatory! Gosu only aims to provide enough code for
// games (or intermediate UI toolkits) to be built upon it.

#include <Gosu/Gosu.hpp>
#include <boost/scoped_ptr.hpp>
#include <cwctype>
#include <algorithm>

class TextField : public Gosu::TextInput
{
    Gosu::Window& window;
    Gosu::Font& font;
    double x, y;
    
public:
    // Some constants that define our appearance.
    // (Can't use Gosu::Color that easily as a class constant, thanks to C++.)
    static const unsigned long INACTIVE_COLOR  = 0xcc666666;
    static const unsigned long ACTIVE_COLOR    = 0xccff6666;
    static const unsigned long SELECTION_COLOR = 0xcc0000ff;
    static const unsigned long CARET_COLOR     = 0xffffffff;
    static const int PADDING = 5;
    
    TextField(Gosu::Window& window, Gosu::Font& font, double x, double y)
    : window(window), font(font), x(x), y(y)
    {
        // Start with a self-explanatory text in each field.
        setText(L"Click to change text");
    
    }

    // Local helper.
    static wchar_t toUpper(wchar_t in)
    {
        // In case towupper is not in std::, we use a 'using' here.
        // (It's actually a C99 function.)
        using namespace std;
        return towupper((wint_t)in);
    }
    
    // Example filter member function. You can truncate the text to employ a length limit,
    // limit the text to certain characters etc.
    std::wstring filter(const std::wstring& string) const
    {
        std::wstring result;
        result.resize(string.length());
        std::transform(string.begin(), string.end(), result.begin(), toUpper);
        return result;
    }
    
    void draw() const
    {
        // Depending on whether this is the currently selected input or not, change the
        // background's color.
        Gosu::Color backgroundColor;
        if (window.input().textInput() == this)
            backgroundColor = ACTIVE_COLOR;
        else
            backgroundColor = INACTIVE_COLOR;
        window.graphics().drawQuad(x - PADDING,           y - PADDING,            backgroundColor,
                                   x + width() + PADDING, y - PADDING,            backgroundColor,
                                   x - PADDING,           y + height() + PADDING, backgroundColor,
                                   x + width() + PADDING, y + height() + PADDING, backgroundColor, 0);
    
        // Calculate the position of the caret and the selection start.
        double posX = x + font.textWidth(text().substr(0, caretPos()));
        double selX = x + font.textWidth(text().substr(0, selectionStart()));

        // Draw the selection background, if any; if not, sel_x and pos_x will be
        // the same value, making this quad empty.
        window.graphics().drawQuad(selX, y,            SELECTION_COLOR,
                                   posX, y,            SELECTION_COLOR,
                                   selX, y + height(), SELECTION_COLOR,
                                   posX, y + height(), SELECTION_COLOR, 0);

        // Draw the caret; again, only if this is the currently selected field.
        if (window.input().textInput() == this)
          window.graphics().drawLine(posX, y,            CARET_COLOR,
                                     posX, y + height(), CARET_COLOR, 0);

        // Finally, draw the text itself!
        font.draw(text(), x, y, 0);
    }
    
    // This text field grows with the text that's being entered.
    // (Usually one would use beginClipping/endClipping and scroll around on the text field.)
    double width() const
    {
        return font.textWidth(text());
    }
    
    double height() const
    {
        return font.height();
    }
    
    bool isUnderPoint(double mouseX, double mouseY)
    {
        return mouseX > x - PADDING && mouseX < x + width() + PADDING &&
            mouseY > y - PADDING and mouseY < y + height() + PADDING;
    }
};

// Helper magic to get size of static array, MUCH safer than sizeof hackery.
template<typename T, std::size_t Len>
std::size_t lengthof(const T(&) [Len])
{
    return Len;
}

class TextInputWindow : public Gosu::Window
{
    boost::scoped_ptr<Gosu::Font> font;
    boost::scoped_ptr<TextField> textFields[3];
    boost::scoped_ptr<Gosu::Image> cursor;

public:
    TextInputWindow()
    : Gosu::Window(300, 200, false)
    {
        setCaption(L"Text Input Example");
    
        font.reset(new Gosu::Font(graphics(), Gosu::defaultFontName(), 20));
        
        for (int index = 0; index < lengthof(textFields); ++index)
            textFields[index].reset(new TextField(*this, *font, 50, 30 + index * 50));
        
        cursor.reset(new Gosu::Image(graphics(), L"media/Cursor.png", false));
    }

    void draw()
    {
        for (int i = 0; i < lengthof(textFields); ++i)
            textFields[i]->draw();
            
        cursor->draw(input().mouseX(), input().mouseY(), 0);
    }
  
    void buttonDown(Gosu::Button btn)
    {
        if (btn == Gosu::kbTab)
        {
            // Tab key will not be 'eaten' by text fields; use for switching through
            // text fields.
            int index = -1;
            for (int i = 0; i < lengthof(textFields); ++i)
                if (input().textInput() == textFields[i].get())
                    index = i;
            input().setTextInput(textFields[(index + 1) % lengthof(textFields)].get());
        }
        else if (btn == Gosu::kbEscape)
        {
            // Escape key will not be 'eaten' by text fields; use for deselecting.
            if (input().textInput())
                input().setTextInput(0);
            else
                close();
        }
        else if (btn == Gosu::msLeft)
        {
            // Mouse click: Select text field based on mouse position.
            input().setTextInput(0);
            for (int i = 0; i < lengthof(textFields); ++i)
                if (textFields[i]->isUnderPoint(input().mouseX(), input().mouseY()))
                    input().setTextInput(textFields[i].get());
        }
    }
};

int main(int argc, char* argv[])
{
    TextInputWindow win;
    win.show();
    return 0;
}
