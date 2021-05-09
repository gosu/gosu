#ifdef ENABLE_FFMPEG

#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#ifdef __cplusplus
}
#endif

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