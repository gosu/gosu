#include <Gosu/Audio.hpp>

// Just stubs for now.

Gosu::Audio::Audio()
{
}

Gosu::Audio::~Audio()
{
}

Gosu::SampleInstance::SampleInstance(int handle, int extra)
: handle(handle), extra(extra)
{
}

bool Gosu::SampleInstance::playing() const
{
    return false;
}

bool Gosu::SampleInstance::paused() const
{
    return false;
}

void Gosu::SampleInstance::pause()
{
}

void Gosu::SampleInstance::resume()
{
}

void Gosu::SampleInstance::stop()
{
}

void Gosu::SampleInstance::changeVolume(double volume)
{
}

void Gosu::SampleInstance::changePan(double pan)
{
}

void Gosu::SampleInstance::changeSpeed(double speed)
{
}

struct Gosu::Sample::SampleData
{
};

Gosu::Sample::Sample(Audio& audio, const std::wstring& filename)
{
}

Gosu::Sample::Sample(Audio& audio, Reader reader)
{
}

Gosu::Sample::~Sample()
{
}

Gosu::SampleInstance Gosu::Sample::play(double volume, double speed,
    bool looping) const
{
    return SampleInstance(0, 0);
}

Gosu::SampleInstance Gosu::Sample::playPan(double pan, double volume,
    double speed, bool looping) const
{
    return SampleInstance(0, 0);
}

struct Gosu::Song::BaseData
{
};

Gosu::Song::Song(Audio& audio, const std::wstring& filename)
{
}

Gosu::Song::Song(Audio& audio, Type type, Reader reader)
{
}

Gosu::Song::~Song()
{
}

Gosu::Song* Gosu::Song::currentSong()
{
    return 0;
}

void Gosu::Song::play(bool looping)
{
}

void Gosu::Song::pause()
{
}

bool Gosu::Song::paused() const
{
    return false;
}

void Gosu::Song::stop()
{
}

bool Gosu::Song::playing() const
{
    return false;
}

double Gosu::Song::volume() const
{
    return 0;
}

void Gosu::Song::changeVolume(double volume)
{
}
