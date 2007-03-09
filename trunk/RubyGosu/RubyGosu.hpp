#ifndef RUBYGOSU_RUBYGOSU_HPP
#define RUBYGOSU_RUBYGOSU_HPP

#include <string>
#include <ruby.h>
// ruby.h is a fucking macro hell. It might be necessary to use more #undefs
// here.
#undef close
#undef sleep
#undef bind
#undef accept
#undef send
#undef read
#undef write

namespace RubyGosu
{
    typedef VALUE (*RubyMethod)(...);

    std::string valueToString(VALUE val);
}

#endif
