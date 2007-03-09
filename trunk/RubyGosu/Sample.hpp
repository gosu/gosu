#ifndef RUBYGOSU_SAMPLE_HPP
#define RUBYGOSU_SAMPLE_HPP

#include <RubyGosu/RubyGosu.hpp>

namespace RubyGosu
{
    void registerSample(VALUE module);
    VALUE Sample_initialize(VALUE self, VALUE windowV, VALUE filenameV);
    VALUE Sample_play(int argc, VALUE *argv, VALUE self);
    VALUE Sample_play_pan(int argc, VALUE *argv, VALUE self);
}

#endif
