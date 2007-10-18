#include <Gosu/Gosu.hpp>
#include <Gosu/AutoLink.hpp> // Makes life easier for Windows users compiling this.

#include <boost/scoped_ptr.hpp> // Pointers used throughout Gosu and this tutorial.
#include <boost/shared_ptr.hpp> // Read about them, they're moving into standard C++! :)

#include <boost/lexical_cast.hpp> // Could also use <sstream>, just for int <-> string conversion

#include <cmath>
#include <cstdlib>
#include <list>
#include <vector>

enum ZOrder
{
    zBackground,
    zStars,
    zPlayer,
    zUI
};

typedef std::vector<boost::shared_ptr<Gosu::Image> > Animation;

class Star
{
    Animation* animation;
    Gosu::Color color;
    double posX, posY;

public:
    explicit Star(Animation& anim)
    {
        animation = &anim;
	
        color.setAlpha(255);
        double red = Gosu::random(40, 255);
        color.setRed(static_cast<Gosu::Color::Channel>(red));
        double green = Gosu::random(40, 255);
        color.setGreen(static_cast<Gosu::Color::Channel>(green));
        double blue = Gosu::random(40, 255);
        color.setBlue(static_cast<Gosu::Color::Channel>(blue));

        posX = Gosu::random(0, 640);
        posY = Gosu::random(0, 480);
    }

    double x() const { return posX; }
    double y() const { return posY; }

    void draw() const
    {
        Gosu::Image& image = 
            *animation->at(Gosu::milliseconds() / 100 % animation->size());

        image.draw(posX - image.width() / 2.0, posY - image.height() / 2.0,
            zStars, 1, 1, color, Gosu::amAdditive);
    }
};

class Player
{
    boost::scoped_ptr<Gosu::Image> image;
    boost::scoped_ptr<Gosu::Sample> beep;
    double posX, posY, velX, velY, angle;
    unsigned score;


public:
    Player(Gosu::Graphics& graphics, Gosu::Audio& audio)
    {
        std::wstring filename = Gosu::sharedResourcePrefix() + L"media/Starfighter.bmp";
        image.reset(new Gosu::Image(graphics, filename));

        filename = Gosu::sharedResourcePrefix() + L"media/Beep.wav";
        beep.reset(new Gosu::Sample(audio, filename));

        posX = posY = velX = velY = angle = 0;
        score = 0;
    }

    unsigned getScore() const
    {
        return score;
    }

    void warp(double x, double y)
    {
        posX = x;
        posY = y;
    }

    void turnLeft()
    {
        angle -= 4.5;
    }

    void turnRight()
    {
        angle += 4.5;
    }

    void accelerate()
    {
        velX += Gosu::offsetX(angle, 0.5);
        velY += Gosu::offsetY(angle, 0.5);
    }

    void move()
    {
        posX += velX;
        while (posX < 0)
            posX += 640;
        while (posX > 640)
            posX -= 640;

        posY += velY;
        while (posY < 0)
            posY += 480;
        while (posY > 480)
            posY -= 480;

        velX *= 0.95;
        velY *= 0.95;
    }

    void draw() const
    {
        image->drawRot(posX, posY, zPlayer, angle);
    }
    
    void collectStars(std::list<Star>& stars)
    {
        std::list<Star>::iterator cur = stars.begin();
        while (cur != stars.end())
        {
            if (Gosu::distance(posX, posY, cur->x(), cur->y()) < 35)
            {
                cur = stars.erase(cur);
                score += 10;
                beep->play();
            }
            else
                ++cur;
        }
    }
};

class GameWindow : public Gosu::Window
{
    boost::scoped_ptr<Gosu::Image> backgroundImage;
    Animation starAnim;
    Gosu::Font font;

    Player player;
    std::list<Star> stars;

public:
    GameWindow()
    : Window(640, 480, false),
        font(graphics(), Gosu::defaultFontName(), 20),
        player(graphics(), audio())
    {
        setCaption(L"Gosu Tutorial Game");

        std::wstring filename = Gosu::sharedResourcePrefix() + L"media/Space.png";
        backgroundImage.reset(new Gosu::Image(graphics(), filename, false));

        filename = Gosu::sharedResourcePrefix() + L"media/Star.png";
        Gosu::imagesFromTiledBitmap(graphics(), filename, 25, 25, false, starAnim);

        player.warp(320, 240);
    }

    void update()
    {
        if (input().down(Gosu::kbLeft) || input().down(Gosu::gpLeft))
            player.turnLeft();
        if (input().down(Gosu::kbRight) || input().down(Gosu::gpRight))
            player.turnRight();
        if (input().down(Gosu::kbUp) || input().down(Gosu::gpButton0))
              player.accelerate();
        player.move();
        player.collectStars(stars);

        if (std::rand() % 25 == 0 && stars.size() < 25)
            stars.push_back(Star(starAnim));
    }

    void draw()
    {
        player.draw();
        backgroundImage->draw(0, 0, zBackground);

        for (std::list<Star>::const_iterator i = stars.begin();
            i != stars.end(); ++i)
        {
            i->draw();
        }

        font.draw(L"Score: " + boost::lexical_cast<std::wstring>(player.getScore()),
            10, 10, zUI, 1, 1, Gosu::Colors::yellow);
    }

    void buttonDown(Gosu::Button btn)
    {
        if (btn == Gosu::kbEscape)
           close();
    }
};

int main(int argc, char* argv[])
{
    GameWindow window;
    window.show();
}
