#include <Gosu/Gosu.hpp>
#include <boost/scoped_ptr.hpp>

class TextInputWindow : public Gosu::Window
{
    boost::scoped_ptr<Gosu::Font> font;
    boost::scoped_ptr<Gosu::TextInput> textInput;

public:
    TextInputWindow()
    : Gosu::Window(300, 200, false)
    {
        setCaption(L"Text Input Example");
        font.reset(new Gosu::Font(graphics(), Gosu::defaultFontName(), 20));
        textInput.reset(new Gosu::TextInput());
        input().setTextInput(textInput.get());
    }
    
    void draw()
    {
        std::wstring text = textInput->text();
        double selX = 10 + font->textWidth(text.substr(0, textInput->selectionStart()));
        double posX = 10 + font->textWidth(text.substr(0, textInput->caretPos()));
        
        graphics().drawQuad(selX, 100, 0xcc0000ff, posX, 100, 0xcc0000ff,
                            selX, 120, 0xcc0000ff, posX, 120, 0xcc0000ff, 0);
                 
        graphics().drawLine(posX, 100, 0xffffffff, posX, 120, 0xffffffff, 0);
        
        font->draw(text, 10, 100, 0);
    }
};

int main(int argc, char* argv[])
{
    TextInputWindow win;
    win.show();
    return 0;
}
