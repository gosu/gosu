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
#include <string>
#include <vector>

enum ZOrder
{
    zBackground,
    zStars,
    zPlayer,
    zUI
};

typedef std::vector<Gosu::Image> Animation;

class Star
{
    Animation animation;
    Gosu::Color color;
    double posX, posY;

public:
    explicit Star(Animation animation)
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
        const Gosu::Image& image =
            animation.at(Gosu::milliseconds() / 100 % animation.size());

        image.draw(posX - image.width() / 2.0, posY - image.height() / 2.0,
            zStars, 1, 1, color, Gosu::amAdd);
    }
};

class Player
{
    Gosu::Image image;
    Gosu::Sample beep;
    double posX, posY, velX, velY, angle;
    unsigned score;

public:
    Player()
    :   image(Gosu::resourcePrefix() + L"media/Starfighter.bmp"),
        beep(Gosu::resourcePrefix() + L"media/Beep.wav")
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
        posX = Gosu::wrap(posX + velX, 0.0, 640.0);
        posY = Gosu::wrap(posY + velY, 0.0, 480.0);

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
    :   Window(640, 480), font(20)
    {
        setCaption(L"Gosu Tutorial Game");

        std::wstring filename = Gosu::resourcePrefix() + L"media/Space.png";
        backgroundImage.reset(new Gosu::Image(filename, Gosu::ifTileable));

        filename = Gosu::resourcePrefix() + L"media/Star.png";
        starAnim = Gosu::loadTiles(filename, 25, 25);

        player.warp(320, 240);
    }

    void update()
    {
        if (Gosu::Input::down(Gosu::kbLeft) || Gosu::Input::down(Gosu::gpLeft))
            player.turnLeft();
        if (Gosu::Input::down(Gosu::kbRight) || Gosu::Input::down(Gosu::gpRight))
            player.turnRight();
        if (Gosu::Input::down(Gosu::kbUp) || Gosu::Input::down(Gosu::gpButton0))
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

        for (Star& star : stars)
            star.draw();

        font.draw(L"Score: " + std::to_wstring(player.getScore()), 10, 10, zUI, 1, 1, Gosu::Color::YELLOW);
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
