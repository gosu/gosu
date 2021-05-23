#ifdef GOSU_ENABLE_VIDEO

#pragma once

#include <Gosu/Bitmap.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/ImageData.hpp>
#include <Gosu/Math.hpp>

#include <cassert>
#include <string>

namespace Gosu
{
    class Video
    {
        struct Impl;
        std::shared_ptr<Impl> pimpl;

    public:
        // Prevent concurrent modification
        static void lock_all();
        static void unlock_all();

        Video(const std::string& filename, unsigned image_flags);
        Video(const std::string& filename, double width, double height, unsigned image_flags);

        virtual ~Video();

        unsigned width() const;

        unsigned height() const;

        void draw(double x, double y, ZPos z, double scale_x, double scale_y, Color c,
                  BlendMode mode) const;

        void draw_rot(double x, double y, ZPos z, double angle, double center_x, double center_y,
                      double scale_x, double scale_y, Color c, BlendMode mode) const;
        void draw_as_quad(double x1, double y1, unsigned color1, double x2, double y2,
                          unsigned color2, double x3, double y3, unsigned color3, double x4,
                          double y4, unsigned color4, double z, BlendMode mode) const;

        void play(bool looping = false);

        void pause();

        bool paused() const;

        void stop();

        bool playing() const;

        double volume() const;

        void set_volume(double volume);

        double length() const;

        double position() const;

        void set_position(double position);

        void update();

        void lock();

        void unlock();

        // Current video frame as Gosu::Image
        Gosu::Image image();
    };
};

#endif