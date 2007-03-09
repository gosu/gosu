#include <Gosu/AutoLink.hpp>
#include <Gosu/AutoLinkMain.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Window.hpp>

#include <boost/scoped_ptr.hpp>

class GameWindow : public Gosu::Window
{
    boost::scoped_ptr<Gosu::Image> backgroundImage;

public:
    GameWindow()
    : Window(640, 480, false, 20)
    {
        setCaption(L"Gosu Tutorial Game");

        Gosu::Buffer backgroundFile;
        Gosu::loadFile(backgroundFile, L"media/Space.png");
        Gosu::Bitmap bitmap;
        Gosu::loadFromPNG(bitmap, backgroundFile.frontReader());
        backgroundImage.reset(new Gosu::Image(graphics(), bitmap));
    }

    void update()
    {
    }

    void draw()
    {
        backgroundImage->draw(0, 0, 0);
    }
};

int main(int argc, char* argv[])
{
    GameWindow window;
    window.show();
    return 0;
}
