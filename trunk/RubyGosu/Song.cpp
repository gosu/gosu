#include <RubyGosu/Song.hpp>
#include <RubyGosu/Window.hpp>
#include <RubyGosu/DataWrapper.hpp>
#include <RubyGosu/Exceptions.hpp>
#include <Gosu/Audio.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/Window.hpp>

// TODO: check for nullness
typedef boost::scoped_ptr<Gosu::Song> SongPtr;

void RubyGosu::registerSong(VALUE module)
{
    VALUE cSong = rb_define_class_under(module, "Song", rb_cObject);
    defineDefaultNew<SongPtr>(cSong);
    rb_define_method(cSong, "initialize", (RubyMethod)Song_initialize, 3);
    rb_define_method(cSong, "play", (RubyMethod)Song_play, 0);
    rb_define_method(cSong, "stop", (RubyMethod)Song_stop, 0);
    rb_define_method(cSong, "playing?", (RubyMethod)Song_playing, 0);
}

VALUE RubyGosu::Song_initialize(VALUE self, VALUE windowV, VALUE filenameV,
    VALUE typeV)
{
    TRY_CPP

    Gosu::Song::Type songType;
    std::string type = valueToString(typeV);
    if (type == "module")
        songType = Gosu::Song::stModule;
    else if (type == "stream")
        songType = Gosu::Song::stStream;
    else
        rb_raise(rb_eArgError, ("Invalid stream type: " + type).c_str());

    Gosu::Buffer buf;
    Gosu::loadFile(buf, Gosu::widen(StringValueCStr(filenameV)));

    get<SongPtr>(self).reset(
        new Gosu::Song(getWindow(windowV).audio(), songType, buf.frontReader()));

    rb_iv_set(self, "@window", windowV);

    return self;

    TRANSLATE_TO_RB
}

VALUE RubyGosu::Song_play(VALUE self)
{
    TRY_CPP

    get<SongPtr>(self)->play();
    return Qnil;

    TRANSLATE_TO_RB
}

VALUE RubyGosu::Song_stop(VALUE self)
{
    TRY_CPP

    get<SongPtr>(self)->stop();
    return Qnil;

    TRANSLATE_TO_RB
}

VALUE RubyGosu::Song_playing(VALUE self)
{
    TRY_CPP

    return get<SongPtr>(self)->playing() ? Qtrue : Qfalse;

    TRANSLATE_TO_RB
}
