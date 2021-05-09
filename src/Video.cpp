// TODO: Change to #ifdef when done implementing!
#ifndef USE_FFMPEG

#include <Gosu/Graphics.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/Video.hpp>

#include <algorithm>
#include <cassert>
#include <cstdlib>

struct Gosu::Video::Impl
{
    double volume_ = 1.0;
    bool looping_ = false;

    void apply_volume()
    {
      // Do stuff
      // volume();
    }

public:
    explicit Impl(const std::string& filename, unsigned image_flags)
    {
        // TODO: create a couple of Gosu::Image/Bitmap's to swap back and furth
        // as video frames are loaded
    }

    explicit Impl(const std::string& filename, double width, double height, unsigned image_flags)
    {

    }

    // TODO: clean up
    ~Impl()
    {
    }

    void play(bool looping)
    {
        looping_ = looping;
    }

    void pause()
    {
    }

    void stop()
    {
    }

    bool playing()
    {
    }

    bool paused()
    {
    }

    double volume() const
    {
        return volume_;
    }

    void set_volume(double volume)
    {
        volume_ = clamp(volume, 0.0, 1.0);
        apply_volume();
    }
};

Gosu::Video::Video(const std::string& filename, unsigned image_flags)
{
    pimpl.reset(new Impl(filename, imge_flags));
}

Gosu::Video::Video(const std::string& filename, double width, double height, unsigned image_flags)
{
    pimpl.reset(new Impl(filename, width, height, image_flags));
}

unsigned Gosu::Video::width() const
{
}

unsigned Gosu::Video::height() const
{
}

void Gosu::Video::draw(double x, double y, ZPos z, double scale_x, double scale_y, Color c,
                       BlendMode mode) const
{
}

void Gosu::Video::draw_rot(double x, double y, ZPos z, double angle, double center_x,
                           double center_y, double scale_x, double scale_y, Color c,
                           BlendMode mode) const
{
}

void Gosu::Video::play(bool looping)
{
}

void Gosu::Video::pause()
{
}

void Gosu::Video::stop()
{
}

bool Gosu::Video::playing() const
{
}

bool Gosu::Video::paused() const
{
}

double Gosu::Video::volume() const
{
}

void Gosu::Video::set_volume(double volume)
{
}

double Gosu::Video::length() const
{
}

double Gosu::Video::position() const
{
}

void Gosu::Video::set_position(double position)
{
}

void Gosu::Video::update()
{
}

#endif