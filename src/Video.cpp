#ifdef GOSU_ENABLE_VIDEO

#include <Gosu/Video.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

struct Gosu::Video::Impl
{
    struct context
    {
        bool locked = false;
        Gosu::Image* image;
        Gosu::Bitmap* bitmap;
    };

    double volume_ = 1.0;
    bool looping_ = false;
    double duration_;
    unsigned video_width_ = 640, video_height_ = 480;
    unsigned image_width_ = 0, image_height_ = 0;
    unsigned image_flags_ = ImageFlags::IF_SMOOTH;

    struct context ctx_;

    libvlc_instance_t *vlc_instance_;
    libvlc_media_t *vlc_media_;
    libvlc_media_player_t *vlc_media_player_;

    static void* lock_video(void* data, void** p_pixels)
    {
        puts("REACHED lock_video");
        struct context* ctx = (context*) data;
        puts("REACHED ctx");
        ctx->locked = true;
        puts("REACHED LOCKER");
        *p_pixels = ctx->bitmap->data();
        puts("REACHED DATA");

        puts("REACHED LOCK");
        return NULL;
    }

    static void unlock_video(void* data, void* picture, void* const* p_pixels)
    {
        struct context* ctx = (context*) data;
        ctx->locked = false;
        puts("REACHED UNLOCK");

        assert (picture == NULL);
    }

    static void display(void* data, void* picture)
    {
        puts("REACHED DISPLAY");
        struct context* ctx = (context*) data;

        ctx->image = new Gosu::Image(*ctx->bitmap);
    }

    void apply_volume()
    {
        libvlc_audio_set_volume(vlc_media_player_, volume());
    }

    void initialize_video(const std::string filename)
    {
        vlc_instance_ = libvlc_new(0, nullptr);
        vlc_media_ = libvlc_media_new_path(vlc_instance_, filename.c_str());
        vlc_media_player_ = libvlc_media_player_new_from_media(vlc_media_);
        libvlc_media_release(vlc_media_);

        // Start playing for a few microseconds to get metadata
        libvlc_media_player_play(vlc_media_player_);

        libvlc_state_t vlc_media_player_state_ = libvlc_media_player_get_state(vlc_media_player_);

        // Block main thread while VLC buffers media
        while (libvlc_media_player_get_length(vlc_media_player_) <= 0) {
        }

        duration_ = libvlc_media_player_get_length(vlc_media_player_) / 1000.0;
        libvlc_video_get_size(vlc_media_player_, 0, &video_width_, &video_height_);

        libvlc_media_player_stop(vlc_media_player_);

        ctx_.bitmap = new Bitmap();
        ctx_.bitmap->resize(width(), height(), Gosu::Color::BLUE);
        ctx_.image = new Image(*ctx_.bitmap, image_flags_);

        // TODO: Resolve segfault and use this callback instead of making VLC draw to window
        libvlc_video_set_callbacks(vlc_media_player_, lock_video, unlock_video, display, &ctx_);
        libvlc_video_set_format(vlc_media_player_, "RGBA", width(), height(), video_width_ * 4);
        // VLC 4 function for OpenGL: https://github.com/videolan/vlc/blob/master/doc/libvlc/sdl_opengl_player.cpp
        // libvlc_video_set_output_callbacks(vlc_media_player_);
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
        return ctx_.image;// new Gosu::Image(Bitmap(width(), height(), Gosu::Color::BLUE));
    }

    void play(bool looping)
    {
        looping_ = looping;
        libvlc_media_player_play(vlc_media_player_);
    }

    void pause()
    {
        libvlc_media_player_pause(vlc_media_player_);
    }

    void stop()
    {
        libvlc_media_player_stop(vlc_media_player_);
    }

    bool playing()
    {
        return libvlc_media_player_is_playing(vlc_media_player_);
    }

    bool paused()
    {
        return !playing();
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