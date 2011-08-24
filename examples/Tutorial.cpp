// All of Gosu.
#include <Gosu/Gosu.hpp>
// To safely include std::tr1::shared_ptr
#include <Gosu/TR1.hpp> 
// Makes life easier for Windows users compiling this.
#include <Gosu/AutoLink.hpp>

#include <cmath>
#include <cstdlib>
#include <list>
#include <memory>
#include <sstream> // For int <-> string conversion
#include <vector>

enum ZOrder
{
    zBackground,
    zStars,
    zPlayer,
    zUI
};

typedef std::vector<std::tr1::shared_ptr<Gosu::Image> > Animation;

class Star
{
    Animation& animation;
    Gosu::Color color;
    double posX, posY;

public:
    explicit Star(Animation& animation)
    :   animation(animation)
    {
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
            *animation.at(Gosu::milliseconds() / 100 % animation.size());

        image.draw(posX - image.width() / 2.0, posY - image.height() / 2.0,
            zStars, 1, 1, color, Gosu::amAdditive);
    }
};

class Player
{
    Gosu::Image image;
    Gosu::Sample beep;
    double posX, posY, velX, velY, angle;
    unsigned score;

public:
    Player(Gosu::Graphics& graphics)
    :   image(graphics, Gosu::sharedResourcePrefix() + L"media/Starfighter.bmp"),
        beep(Gosu::sharedResourcePrefix() + L"media/Beep.wav")
    {
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
        posX = Gosu::wrap(posX, 0.0, 640.0);
        posY += velY;
        posY = Gosu::wrap(posY, 0.0, 480.0);

        velX *= 0.95;
        velY *= 0.95;
    }

    void draw() const
    {
        image.drawRot(posX, posY, zPlayer, angle);
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
                beep.play();
            }
            else
                ++cur;
        }
    }
};

class GameWindow : public Gosu::Window
{
    std::auto_ptr<Gosu::Image> backgroundImage;
    Animation starAnim;
    Gosu::Font font;

    Player player;
    std::list<Star> stars;

public:
    GameWindow()
    :   Window(640, 480, false),
        font(graphics(), Gosu::defaultFontName(), 20),
        player(graphics())
    {
        setCaption(L"Gosu Tutorial Game");

        std::wstring filename = Gosu::sharedResourcePrefix() + L"media/Space.png";
        backgroundImage.reset(new Gosu::Image(graphics(), filename, true));

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

        std::wstringstream score;
        score << L"Score: "; 
        score << player.getScore();
        font.draw(score.str(), 10, 10, zUI, 1, 1, Gosu::Colors::yellow);
    }

    void buttonDown(Gosu::Button btn)
    {
        if (btn == Gosu::kbEscape)
           close();
    }
};

int main()
{
    GameWindow window;
    window.show();
}
