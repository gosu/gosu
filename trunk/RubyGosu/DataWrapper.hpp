#ifndef RUBYGOSU_DATAWRAPPER_HPP
#define RUBYGOSU_DATAWRAPPER_HPP

#include <RubyGosu/RubyGosu.hpp>
#include <RubyGosu/Exceptions.hpp>

namespace RubyGosu
{
    template<typename Class>
    void freeObj(void* ptr)
    {
        delete static_cast<Class*>(ptr);
    }

    template<typename Class>
    VALUE Default_new(int argc, VALUE* argv, VALUE cls)
    {
        TRY_CPP

        Class* ptr = new Class;
        VALUE result = Data_Wrap_Struct(cls, 0, freeObj<Class>, ptr);
        // TODO: rescue
        rb_obj_call_init(result, argc, argv);
        return result;

        TRANSLATE_TO_RB
    }

    template<typename Class>
    void defineDefaultNew(VALUE cls)
    {
        typedef VALUE (*ptr)(int, VALUE*, VALUE);
        ptr p = Default_new<Class>;
        rb_define_singleton_method(cls, "new", (RubyMethod)p, -1);
    }

    template<typename Class>
    Class& get(VALUE val)
    {
        Class* ptr;
        Data_Get_Struct(val, Class, ptr);
        return *ptr;
    }
}

#endif
