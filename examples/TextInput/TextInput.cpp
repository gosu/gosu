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
#include <cwctype>
#include <algorithm>
#include <memory>

class TextField : public Gosu::TextInput
{
    Gosu::Window& window;
    Gosu::Font font;
    double x, y;
    
public:
    // Some ARGB constants that define our appearance.
    static const unsigned long INACTIVE_COLOR  = 0xcc666666;
    static const unsigned long ACTIVE_COLOR    = 0xccff6666;
    static const unsigned long SELECTION_COLOR = 0xcc0000ff;
    static const unsigned long CARET_COLOR     = 0xffffffff;
    static const int PADDING = 5;
    
    TextField(Gosu::Window& window, Gosu::Font font, double x, double y)
    : window(window), font(font), x(x), y(y)
    {
        // Start with a self-explanatory text in each field.
        set_text("Click to change text");
    }
    
    // Example filter member function. You can truncate the text to employ a length limit,
    // limit the text to certain characters etc.
    std::string filter(std::string string) const override
    {
        std::wstring wide_string = Gosu::utf8_to_wstring(string);
        std::transform(wide_string.begin(), wide_string.end(), wide_string.begin(), [](wchar_t in) {
            // Use namespace std so it doesn't matter whether towupper is part of it or not.
            // (Depends on your compiler/standard library.)
            using namespace std;
            return towupper((wint_t) in);
        });
        return Gosu::wstring_to_utf8(wide_string);
    }
    
    void draw() const
    {
        Gosu::Color background_color = INACTIVE_COLOR;
        // Highlight this text field if it is currently selected.
        if (window.input().text_input() == this) {
            background_color = ACTIVE_COLOR;
        }
        
        Gosu::Graphics::draw_quad(x - PADDING,           y - PADDING,            background_color,
                                  x + width() + PADDING, y - PADDING,            background_color,
                                  x - PADDING,           y + height() + PADDING, background_color,
                                  x + width() + PADDING, y + height() + PADDING, background_color,
                                  0);
        
        // Calculate the position of the caret and the selection start.
        double pos_x = x + font.text_width(text().substr(0, caret_pos()));
        double sel_x = x + font.text_width(text().substr(0, selection_start()));
        
        // Draw the selection background, if any; if not, sel_x and pos_x will be
        // the same value, making this quad empty.
        Gosu::Graphics::draw_quad(sel_x, y,            SELECTION_COLOR,
                                  pos_x, y,            SELECTION_COLOR,
                                  sel_x, y + height(), SELECTION_COLOR,
                                  pos_x, y + height(), SELECTION_COLOR, 0);

        // Draw the caret; again, only if this is the currently selected field.
        if (window.input().text_input() == this) {
            Gosu::Graphics::draw_line(pos_x, y,            CARET_COLOR,
                                      pos_x, y + height(), CARET_COLOR, 0);
        }
        
        // Finally, draw the text itself!
        font.draw(text(), x, y, 0);
    }
    
    // This text field grows with the text that's being entered.
    // (Usually one would use begin_clipping/end_clipping and scroll around on the text field.)
    double width() const
    {
        return font.text_width(text());
    }
    
    double height() const
    {
        return font.height();
    }
    
    bool is_under_point(double mouse_x, double mouse_y)
    {
        return mouse_x > x - PADDING && mouse_x < x + width() + PADDING &&
               mouse_y > y - PADDING && mouse_y < y + height() + PADDING;
    }
};

// Helper to get the size static arrays.
template<typename T, std::size_t Length>
std::size_t lengthof(const T(&) [Length])
{
    return Length;
}

class Window : public Gosu::Window
{
    std::unique_ptr<TextField> text_fields[3];
    
public:
    Window()
    : Gosu::Window(300, 200)
    {
        set_caption("Text Input Example");
        
        Gosu::Font font(20);
        
        for (int index = 0; index < lengthof(text_fields); ++index) {
            text_fields[index].reset(new TextField(*this, font, 50, 30 + index * 50));
        }
    }
    
    bool needs_cursor() const override
    {
        return true;
    }
    
    void draw() override
    {
        for (auto& text_field : text_fields) {
            text_field->draw();
        }
    }
    
    void button_down(Gosu::Button btn) override
    {
        if (btn == Gosu::KB_TAB) {
            // Tab key will not be 'eaten' by text fields; use for switching through
            // text fields.
            int index = -1;
            for (int i = 0; i < lengthof(text_fields); ++i) {
                if (input().text_input() == text_fields[i].get()) {
                    index = i;
                }
            }
            input().set_text_input(text_fields[(index + 1) % lengthof(text_fields)].get());
        }
        else if (btn == Gosu::KB_ESCAPE) {
            // Escape key will not be 'eaten' by text fields; use for deselecting.
            if (input().text_input()) {
                input().set_text_input(0);
            }
            else {
                close();
            }
        }
        else if (btn == Gosu::MS_LEFT) {
            // Mouse click: Select text field based on mouse position.
            input().set_text_input(0);
            for (int i = 0; i < lengthof(text_fields); ++i) {
                if (text_fields[i]->is_under_point(input().mouse_x(), input().mouse_y())) {
                    input().set_text_input(text_fields[i].get());
                }
            }
        }
    }
};

int main(int argc, char* argv[])
{
    Window window;
    window.show();
    return 0;
}
