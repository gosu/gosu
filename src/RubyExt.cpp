/*
** (C) 2019 Kyonides Arkanthes <kyonides@gmail.com>
**
** Usage:
**
** module CustomModule
**   module_accessor :my_method
**   module_accessor :some_method, :just_another_method, :"what_s_up"
** end
**
** It's the same as:
** class << self
*    attr_accessor :some_method, :just_another_method, :"what_s_up"
** end
**
** The same is valid for module_reader and module_writer
**
** Aliases: mod_reader, mod_writer, mod_accessor
*/

#include <ruby.h>

static VALUE hc_sym(const char* str)
{
    return rb_id2sym(rb_intern(str));
}

static VALUE hc_sym2(VALUE str)
{
    return rb_id2sym(rb_intern_str(str));
}

static VALUE module_reader(int argc, VALUE* argv, VALUE self)
{
    if (!RB_TYPE_P(self, T_MODULE))
        rb_raise(rb_eTypeError, "module_reader is exclusive for modules!");
    VALUE args[argc], basic_str, new_str;
    for (int n = 0 ; n < argc ; n++) {
        basic_str = rb_sym_to_s(argv[n]);
        const char* func_name = StringValueCStr(basic_str);
        rb_define_attr(self, func_name, 1, 0);
    }
    VALUE meth = rb_obj_method(self, hc_sym("module_function"));
    rb_method_call(argc, args, meth);
    return Qnil;
}

static VALUE module_writer(int argc, VALUE* argv, VALUE self)
{
    if (!RB_TYPE_P(self, T_MODULE))
        rb_raise(rb_eTypeError, "module_writer is exclusive for modules!");
    VALUE args[argc], basic_str, new_str;
    for (int n = 0 ; n < argc ; n++) {
        basic_str = rb_sym_to_s(argv[n]);
        const char* func_name = StringValueCStr(basic_str);
        new_str = rb_str_plus(basic_str, rb_str_new_cstr("="));
        args[n] = hc_sym2(new_str);
        rb_define_attr(self, func_name, 0, 1);
    }
    VALUE meth = rb_obj_method(self, hc_sym("module_function"));
    rb_method_call(argc, args, meth);
    return Qnil;
}

static VALUE module_accessor(int argc, VALUE* argv, VALUE self)
{
    if (!RB_TYPE_P(self, T_MODULE))
        rb_raise(rb_eTypeError, "module_accessor is exclusive for modules!");
    int m, count = argc * 2;
    VALUE args[count], basic_str, new_str;
    for (int n = 0 ; n < argc ; n++) {
        m = n * 2;
        args[m] = argv[n];
        basic_str = rb_sym_to_s(argv[n]);
        const char* func_name = StringValueCStr(basic_str);
        rb_define_attr(self, func_name, 1, 1);
        new_str = rb_str_plus(basic_str, rb_str_new_cstr("="));
        args[m + 1] = hc_sym2(new_str);
    }
    VALUE meth = rb_obj_method(self, hc_sym("module_function"));
    rb_method_call(count, args, meth);
    return Qnil;
}

#define RMF(func) ((VALUE (*)(ANYARGS))(func))

void ruby_ext_init()
{
    rb_define_method(rb_cModule, "module_reader", RMF(module_reader), -1);
    rb_define_method(rb_cModule, "module_writer", RMF(module_writer), -1);
    rb_define_method(rb_cModule, "module_accessor", RMF(module_accessor), -1);
    rb_define_method(rb_cModule, "mod_reader", RMF(module_reader), -1);
    rb_define_method(rb_cModule, "mod_writer", RMF(module_writer), -1);
    rb_define_method(rb_cModule, "mod_accessor", RMF(module_accessor), -1);
}