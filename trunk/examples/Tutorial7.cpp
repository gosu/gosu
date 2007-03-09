#include <Gosu/AutoLink.hpp>
#include <Gosu/AutoLinkMain.hpp>
#include <Gosu/Audio.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Color.hpp> 
#include <Gosu/Directories.hpp> 
#include <Gosu/Font.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Input.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Math.hpp> 
#include <Gosu/Text.hpp>
#include <Gosu/Timing.hpp>
#include <Gosu/Window.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

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
    Animation animation;
    Gosu::Color color;
    double posX, posY;

public:
    explicit Star(const Animation& anim)
    : animation(anim)
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
            *animation[Gosu::milliseconds() / 100 % animation.size()];

        image.draw(posX - image.width() / 2.0, posY - image.height() / 2.0,
            zStars, 1, 1, color, Gosu::amAdditive);
    }
};





class Player
{
	 boost::scoped_ptr<Gosu::Image> image;
    boost::scoped_ptr<Gosu::Sample> beep;
    double posX, posY, velX, velY, angle;

public:
    explicit Player(Gosu::Graphics& graphics, Gosu::Audio& audio)
    {
    	  Gosu::Buffer starfighterFile;
        Gosu::loadFile(starfighterFile, Gosu::sharedResourcePrefix() + L"media/Starfighter.bmp");
        Gosu::Bitmap bitmap;
        Gosu::loadFromBMP(bitmap, starfighterFile.frontReader());
        Gosu::applyColorKey(bitmap, Gosu::Colors::fuchsia);
        image.reset(new Gosu::Image(graphics, bitmap));
        
        Gosu::Buffer beepFile;
        Gosu::loadFile(beepFile, Gosu::sharedResourcePrefix() + L"media/Beep.wav");
        beep.reset(new Gosu::Sample(audio, beepFile.frontReader()));
        
        posX = posY = velX = velY = angle = 0;
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
    
    void collectStars(std::list<Star>& stars, unsigned& score)
    {
        std::list<Star>::iterator cur = stars.begin();
        while (cur != stars.end())
        {
            double distX = posX - cur->x();
            double distY = posY - cur->y();
            double dist = std::sqrt(distX * distX + distY * distY);

            if (dist < 35)
            {
                cur = stars.erase(cur);
                score += 10;
                beep->play();
            }
            else
            {
                ++cur;
            }
            
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
    unsigned score;



public:
    GameWindow()
    : Window(640, 480, false, 20),
        font(graphics(), Gosu::defaultFontName(), 20),
        player(graphics(), audio())
    {
        setCaption(L"Gosu Tutorial Game");
        
        Gosu::Bitmap bitmap;
        Gosu::Buffer backgroundFile;
        Gosu::loadFile(backgroundFile, Gosu::sharedResourcePrefix() + L"media/Space.png");
        Gosu::loadFromPNG(bitmap, backgroundFile.frontReader());
        backgroundImage.reset(new Gosu::Image(graphics(), bitmap));

        Gosu::Buffer starFile;
        Gosu::loadFile(starFile, Gosu::sharedResourcePrefix() + L"media/Star.png");
        Gosu::loadFromPNG(bitmap, starFile.frontReader());
        Gosu::imagesFromTiledBitmap(graphics(), bitmap, 25, 25, false, starAnim);

        player.warp(320, 240);
        score = 0;
    }
    
    void buttonDown(unsigned id)
    {
        if (id == Gosu::kbEscape)
            close();
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
        player.collectStars(stars, score);

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

        font.draw(L"Score: " + boost::lexical_cast<std::wstring>(score),
            10, 10, zUI, 1, 1, Gosu::Colors::yellow);
        
    }
};

int main(int argc, char* argv[])
{
    GameWindow window;
    window.show();
    return 0;
}
