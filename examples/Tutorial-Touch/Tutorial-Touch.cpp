// All of Gosu.
#include <Gosu/Gosu.hpp>

#include <cmath>
#include <cstdlib>
#include <list>
#include <memory>
#include <string>
#include <vector>

enum ZOrder
{
    Z_BACKGROUND,
    Z_STARS,
    Z_PLAYER,
    Z_UI
};

static const unsigned WIDTH = 1024, HEIGHT = 768;

typedef std::vector<Gosu::Image> Animation;

class Star
{
    Animation animation;
    Gosu::Color color;
    double pos_x, pos_y;

public:
    explicit Star(Animation animation)
    :   animation(animation)
    {
        color.set_alpha(255);
        double red = Gosu::random(40, 255);
        color.set_red(static_cast<Gosu::Color::Channel>(red));
        double green = Gosu::random(40, 255);
        color.set_green(static_cast<Gosu::Color::Channel>(green));
        double blue = Gosu::random(40, 255);
        color.set_blue(static_cast<Gosu::Color::Channel>(blue));

        pos_x = Gosu::random(0, WIDTH);
        pos_y = Gosu::random(0, HEIGHT);
    }

    double x() const
    {
        return pos_x;
    }
    
    double y() const
    {
        return pos_y;
    }
    
    void draw() const
    {
        const Gosu::Image& image = animation.at(Gosu::milliseconds() / 100 % animation.size());
        
        image.draw(pos_x - image.width() / 2.0, pos_y - image.height() / 2.0, Z_STARS,
                   1, 1, color, Gosu::AM_ADD);
    }
};

class Player
{
    Gosu::Image image;
    Gosu::Sample beep;
    double pos_x, pos_y, vel_x, vel_y, angle;
    unsigned score;

public:
    Player()
    :   image(Gosu::resource_prefix() + "media/Starfighter.bmp"),
        beep(Gosu::resource_prefix() + "media/Beep.wav")
    {
        pos_x = pos_y = vel_x = vel_y = angle = 0;
        score = 0;
    }

    unsigned get_score() const
    {
        return score;
    }
    
    void warp(double x, double y)
    {
        pos_x = x;
        pos_y = y;
    }
    
    void rotate_towards(double x, double y)
    {
        double target_angle = Gosu::angle(pos_x, pos_y, x, y);
        angle = angle + 0.1 * Gosu::angle_diff(angle, target_angle);
    }

    void accelerate()
    {
        vel_x += Gosu::offset_x(angle, 0.5);
        vel_y += Gosu::offset_y(angle, 0.5);
    }

    void move()
    {
        pos_x += vel_x;
        pos_y += vel_y;
        
        vel_x *= 0.95;
        vel_y *= 0.95;
    }

    void draw() const
    {
        image.draw_rot(pos_x, pos_y, Z_PLAYER, angle);
    }

    void collect_stars(std::list<Star>& stars)
    {
        std::list<Star>::iterator cur = stars.begin();
        while (cur != stars.end()) {
            if (Gosu::distance(pos_x, pos_y, cur->x(), cur->y()) < 35) {
                cur = stars.erase(cur);
                score += 10;
                beep.play();
            }
            else {
                ++cur;
            }
        }
    }
};

class GameWindow : public Gosu::Window
{
    std::unique_ptr<Gosu::Image> background_image;
    Animation star_anim;
    Gosu::Font font;

    Player player;
    std::list<Star> stars;

public:
    GameWindow()
    :   Window(WIDTH, HEIGHT), font(20)
    {
        set_caption("Gosu Tutorial Game");
        
        std::string filename = Gosu::resource_prefix() + "media/Space.png";
        background_image.reset(new Gosu::Image(filename, Gosu::IF_TILEABLE));
        
        filename = Gosu::resource_prefix() + "media/Star.png";
        star_anim = Gosu::load_tiles(filename, 25, 25);
        
        player.warp(320, 240);
    }

    void update() override
    {
        if (!input().current_touches().empty()) {
            Gosu::Touch target_touch = input().current_touches().front();
            player.rotate_towards(target_touch.x, target_touch.y);
            player.accelerate();
        }
        player.move();
        player.collect_stars(stars);

        if (std::rand() % 25 == 0 && stars.size() < 25) {
            stars.push_back(Star(star_anim));
        }
    }

    void draw() override
    {
        player.draw();
        background_image->draw(0, 0, Z_BACKGROUND,
                               1.0 * WIDTH / background_image->width(),
                               1.0 * HEIGHT / background_image->height());
        
        for (Star& star : stars) {
            star.draw();
        }

        font.draw("Score: " + std::to_string(player.get_score()), 10, 10, Z_UI,
                  1, 1, Gosu::Color::YELLOW);
    }
};

Gosu::Window &window_instance()
{
    static GameWindow window;
    return window;
}
