// TODO: Change to #ifdef when done implementing!
#ifndef USE_FFMPEG

#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Platform.hpp>
#include <memory>
#include <string>

#include <libavcodec/avcodec.h>

namespace Gosu
{
    class Video
    {
        struct Impl;
        std::shared_ptr<Impl> pimpl;

    public:
        Video(const std::string& filename, unsigned image_flags);
        Video(const std::string& filename, double width, double height, unsigned image_flags);

        ~Video();

        unsigned width() const;

        unsigned height() const;

        void draw(double x, double y, ZPos z, double scale_x, double scale_y, Color c,
                  BlendMode mode) const;

        void draw_rot(double x, double y, ZPos z, double angle, double center_x, double center_y,
                      double scale_x, double scale_y, Color c, BlendMode mode) const;

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
    };
};

#endif