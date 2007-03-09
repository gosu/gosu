#ifndef RUBYGOSU_EXCEPTIONS_HPP
#define RUBYGOSU_EXCEPTIONS_HPP

#include <exception>

namespace RubyGosu
{
    class RubyError
    {
    };
}

#define TRY_CPP try {
#define TRANSLATE_TO_RB } catch (const RubyError& e) { VALUE err = ruby_errinfo; ruby_errinfo = Qnil; rb_exc_raise(err); } catch (const std::exception& e) { rb_raise(rb_eRuntimeError, e.what()); }

#endif
