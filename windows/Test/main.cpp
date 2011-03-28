#include <Gosu/AutoLink.hpp>
#include <Gosu/Gosu.hpp>

#include <iostream>

using namespace std;

class GameWindow : public Gosu::Window
{
private:
    boost::shared_ptr<Gosu::Font> font;
    Gosu::Image *img, *sky;

public:
    GameWindow() : Gosu::Window(1024, 768, false)
    {
        //this->font.reset(new Gosu::Font(graphics(), L"examples/media/RAVIE.TTF", 40));
        this->img = new Gosu::Image(graphics(), L"examples/media/Earth.png");
        this->sky = new Gosu::Image(graphics(), L"examples/media/Sky.jpg");

        //this->p1 = new Gosu::Image(graphics(), L"examples/media/broken_NO_CHECKIN/001-Fighter01.png");
        //this->p2 = new Gosu::Image(graphics(), L"examples/media/broken_NO_CHECKIN/pal_rgba.png");


        /*graphics().beginRecording();
        img->draw(0, 0, 1000, 40, 40);
        this->font->draw(L"Hi ..", 10, 10, 1000);
        this->vertex = new Gosu::Image(graphics().endRecording());*/
    }

    void draw()
    {
        //this->font->draw(L"Yay", 10, 10, 120);

        //graphics().drawQuad(0, 0, Gosu::Color::RED, 500, 0, Gosu::Color::AQUA, 0, 500, Gosu::Color::WHITE, 500, 500, Gosu::Color::YELLOW, 5);
        img->draw(0, 0, 15);
        sky->draw(30, 30, 15);


        //vertex->draw(300, 400, 200);

        //graphics().drawQuad(0, 0, Gosu::Color::RED, 500, 0, Gosu::Color::AQUA, 0, 500, Gosu::Color::WHITE, 500, 500, Gosu::Color::YELLOW, 5);
        //img->draw(0, 0, 15);
        //p1->draw(100, 0, 15);
        //p2->draw(0, 100, 15);


        /*static bool asd = false;

        if(!asd)
        {
            graphics().beginRecording();
            img->draw(0, 0, 1000, 40, 40);
            this->font->draw(L"Hi ..", 10, 10, 1000);
            this->rtt = new Gosu::Image(graphics().endRTT());
            asd = true;
        }

        if(asd)
            rtt->draw(300, 300, 0);*/
        //rtt->draw(300, 300, 0);
    }

    void update()
    {

    }

    void buttonDown(Gosu::Button button)
    {
        if(button == Gosu::kbEscape) close();
    }
};

int main(int argc, char* argv[])
{
    GameWindow win;
    win.show();

    return 0;
}
