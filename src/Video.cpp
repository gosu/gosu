#ifdef GOSU_ENABLE_VIDEO

#include "GraphicsImpl.hpp"
#include <Gosu/Video.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

struct Gosu::Video::Impl
{
    struct context
    {
        bool locked = false;
        Gosu::Bitmap* bitmap;
    };

    double volume_ = 1.0;
    bool looping_ = false;
    double duration_;
    unsigned video_width_ = 0, video_height_ = 0;
    unsigned image_width_ = 0, image_height_ = 0;
    unsigned image_flags_ = ImageFlags::IF_SMOOTH;

    libvlc_instance_t *vlc_instance_;
    libvlc_media_t *vlc_media_;
    libvlc_media_player_t *vlc_media_player_;

    static void* lock_video(void* data, void** p_pixels)
    {
        struct context* ctx = (context *) data;

        ctx->locked = true;
        *p_pixels = ctx->bitmap->data();
        return NULL;
    }

    static void unlock_video(void* data, void* id, void* const* p_pixels)
    {
        struct context* ctx = (context*) data;

        assert(id == NULL);
    }
    static void display(void* data, void* id)
    {

    }

    void apply_volume()
    {
        // Do stuff
        // volume();
    }

    void initialize_video(const std::string filename)
    {
        struct context context;

        vlc_instance_ = libvlc_new(0, NULL);
        vlc_media_ = libvlc_media_new_path(vlc_instance_, filename.c_str());
        vlc_media_player_ = libvlc_media_player_new_from_media(vlc_media_);

        // Start playing for a few microseconds to get metadata
        libvlc_media_player_play(vlc_media_player_);

        libvlc_state_t vlc_media_player_state_ = libvlc_media_player_get_state(vlc_media_player_);

        // Block main thread while VLC buffers media
        while (libvlc_media_player_get_length(vlc_media_player_) <= 0) {
        }

        duration_ = libvlc_media_player_get_length(vlc_media_player_) / 1000.0;
        libvlc_video_get_size(vlc_media_player_, 0, &video_width_, &video_height_);

        libvlc_media_player_stop(vlc_media_player_);

        // TODO: Resolve segfault use this callback instead of making VLC draw to window
        // libvlc_video_set_callbacks(vlc_media_player_, lock_video, nullptr, nullptr, nullptr);
        // libvlc_video_set_format(vlc_media_player_, "RGBA", video_width_, video_height_, video_width_ * 4);
    }

public:
    explicit Impl(const std::string& filename, unsigned image_flags)
    {
        // TODO: create a couple of Gosu::Image/Bitmap's to swap back and furth
        // as video frames are loaded

        image_flags_ = image_flags;

        initialize_video(filename);
    }

    explicit Impl(const std::string& filename, double width, double height, unsigned image_flags)
    {
        image_width_ = width;
        image_height_ = height;
        image_flags_ = image_flags;

        initialize_video(filename);
    }

    // TODO: clean up
    ~Impl()
    {
        libvlc_media_player_stop(vlc_media_player_);
        libvlc_release(vlc_instance_);
    }

    unsigned width()
    {
        return video_width_;
    }

    unsigned height()
    {
        return video_height_;
    }

    Gosu::Image* image()
    {
        return new Gosu::Image(Bitmap(width(), height(), Gosu::Color::BLUE));
    }

    void play(bool looping)
    {
        looping_ = looping;
        libvlc_media_player_set_xwindow(vlc_media_player_, SDL_GetWindowID(Gosu::shared_window()));
        libvlc_media_player_play(vlc_media_player_);
    }

    void pause()
    {
    }

    void stop()
    {
        libvlc_media_player_set_xwindow(vlc_media_player_, 0);
        libvlc_media_player_stop(vlc_media_player_);
    }

    bool playing()
    {
        return false;
    }

    bool paused()
    {
        return false;
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

    double length()
    {
        return duration_;
    }

    double position()
    {
        return 0;
    }

    void set_position(double position)
    {
    }

    void update()
    {
        // Feed frames
        // Feed audio
    }
};

Gosu::Video::Video(const std::string& filename, unsigned image_flags)
{
    pimpl.reset(new Impl(filename, image_flags));
}

Gosu::Video::Video(const std::string& filename, double width, double height, unsigned image_flags)
{
    pimpl.reset(new Impl(filename, width, height, image_flags));
}

unsigned Gosu::Video::width() const
{
    return pimpl->width();
}

unsigned Gosu::Video::height() const
{
    return pimpl->height();
}

void Gosu::Video::draw(double x, double y, ZPos z, double scale_x, double scale_y, Color c,
                       BlendMode mode) const
{
    pimpl->image()->draw(x, y, z, scale_x, scale_y, c, mode);
}

void Gosu::Video::draw_rot(double x, double y, ZPos z, double angle, double center_x,
                           double center_y, double scale_x, double scale_y, Color c,
                           BlendMode mode) const
{
    pimpl->image()->draw_rot(x, y, z, angle, center_x, center_y, scale_x, scale_y, c, mode);
}

void Gosu::Video::draw_as_quad(double x1, double y1, unsigned color1, double x2, double y2,
                               unsigned color2, double x3, double y3, unsigned color3, double x4,
                               double y4, unsigned color4, double z, BlendMode mode) const
{
    pimpl->image()->data().draw(x1, y1, color1, x2, y2, color2, x3, y3, color3, x4, y4, color4, z,
                                mode);
}

void Gosu::Video::play(bool looping)
{
    pimpl->play(looping);
}

void Gosu::Video::pause()
{
    pimpl->pause();
}

void Gosu::Video::stop()
{
    pimpl->stop();
}

bool Gosu::Video::playing() const
{
    return pimpl->playing();
}

bool Gosu::Video::paused() const
{
    return pimpl->paused();
}

double Gosu::Video::volume() const
{
    return pimpl->volume();
}

void Gosu::Video::set_volume(double volume)
{
    pimpl->set_volume(volume);
}

double Gosu::Video::length() const
{
    return pimpl->length();
}

double Gosu::Video::position() const
{
    return pimpl->position();
}

void Gosu::Video::set_position(double position)
{
    pimpl->set_position(position);
}

void Gosu::Video::update()
{
    return pimpl->update();
}

Gosu::Image* image()
{
    return image();
}

#endif