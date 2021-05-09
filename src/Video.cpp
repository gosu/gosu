#ifdef ENABLE_FFMPEG

#include <Gosu/Video.hpp>

struct Gosu::Video::Impl
{
    double volume_ = 1.0;
    bool looping_ = false;
    int image_width_ = 0, image_height_ = 0;
    unsigned image_flags_ = ImageFlags::IF_SMOOTH;

    AVFormatContext* fmt_ctx = nullptr;
    AVDictionaryEntry* tag = nullptr;

    const AVCodec *codec;
    AVCodecParser *parser;
    AVCodecParserContext *context = nullptr;

    void apply_volume()
    {
      // Do stuff
      // volume();
    }

    void initialize_video(const std::string filename)
    {
        // First thing to do is check if file exists, then if we can actually load/play it (supported codec)
        int ret;

        printf("Video file: %s\n", filename.c_str());

        if ((ret = avformat_open_input(&fmt_ctx, filename.c_str(), NULL, NULL))) return;

        puts("Successfully opened input!");

        if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
            return;
        }

        puts("Successfully read stream info");

        while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
            printf("%s=%s\n", tag->key, tag->value);

        avformat_close_input(&fmt_ctx);
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
    pimpl.reset(new Impl(filename, image_flags));
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