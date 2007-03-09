#include <Gosu/AutoLink.hpp>
#include <Gosu/AutoLinkMain.hpp>
#include <Gosu/Window.hpp>

class GameWindow : public Gosu::Window
{
public:
    GameWindow()
    : Window(640, 480, false, 20)
    {
        setCaption(L"Gosu Tutorial Game");
    }

    void update()
    {
    }

    void draw()
    {
    }
};

int main(int argc, char* argv[])
{
    GameWindow window;
    window.show();
    return 0;
}
