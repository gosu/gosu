#include <Gosu/AutoLink.hpp>
#include <Gosu/AutoLinkMain.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Window.hpp>
#include <Gosu/Color.hpp> 
#include <Gosu/Directories.hpp> 
#include <Gosu/Math.hpp> 
#include <boost/scoped_ptr.hpp>

class Player
{
	 boost::scoped_ptr<Gosu::Image> image;
    double posX, posY, velX, velY, angle;

public:
    explicit Player(Gosu::Graphics& graphics)
    {
    	  Gosu::Buffer starfighterFile;
        Gosu::loadFile(starfighterFile, Gosu::sharedResourcePrefix() + L"media/Starfighter.bmp");
        Gosu::Bitmap bitmap;
        Gosu::loadFromBMP(bitmap, starfighterFile.frontReader());
        Gosu::applyColorKey(bitmap, Gosu::Colors::fuchsia);
        image.reset(new Gosu::Image(graphics, bitmap));
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
        image->drawRot(posX, posY, 1, angle);
    }
    
    
};

  



class GameWindow : public Gosu::Window
{
    boost::scoped_ptr<Gosu::Image> backgroundImage;
	 Player player;

public:
    GameWindow()
    : Window(640, 480, false, 20), player(graphics())
    {
        setCaption(L"Gosu Tutorial Game");
        player.warp(320, 240);

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
    	  player.draw();
        backgroundImage->draw(0, 0, 0);
        
    }
};

int main(int argc, char* argv[])
{
    GameWindow window;
    window.show();
    return 0;
}
