#include "Gosu_FFI_internal.h"

GOSU_FFI_API Gosu_TextInput* Gosu_TextInput_create()
{
    return Gosu_translate_exceptions([=] {
        return new Gosu_TextInput{};
    });
}

GOSU_FFI_API void Gosu_TextInput_destroy(Gosu_TextInput* text_input)
{
    delete text_input;
}

GOSU_FFI_API const char* Gosu_TextInput_text(Gosu_TextInput* text_input)
{
    static thread_local std::string text;

    return Gosu_translate_exceptions([=] {
        text = text_input->text();
        return text.c_str();
    });
}

GOSU_FFI_API void Gosu_TextInput_set_text(Gosu_TextInput* text_input, const char* text)
{
    Gosu_translate_exceptions([=] {
        text_input->set_text(text);
    });
}

GOSU_FFI_API int Gosu_TextInput_caret_pos(Gosu_TextInput* text_input)
{
    return Gosu_translate_exceptions([=] {
        return text_input->caret_pos();
    });
}

GOSU_FFI_API void Gosu_TextInput_set_caret_pos(Gosu_TextInput* text_input, int pos)
{
    Gosu_translate_exceptions([=] {
        text_input->set_caret_pos(pos);
    });
}

GOSU_FFI_API int Gosu_TextInput_selection_start(Gosu_TextInput* text_input)
{
    return Gosu_translate_exceptions([=] {
        return text_input->selection_start();
    });
}

GOSU_FFI_API void Gosu_TextInput_set_selection_start(Gosu_TextInput* text_input, int pos)
{
    Gosu_translate_exceptions([=] {
        text_input->set_selection_start(pos);
    });
}

GOSU_FFI_API void Gosu_TextInput_set_filter(Gosu_TextInput* text_input,
                                            void function(void*, const char*), void* data)
{
    Gosu_translate_exceptions([=] {
        text_input->filter_callback = [=](const char* text) { function(data, text); };
    });
}

GOSU_FFI_API void Gosu_TextInput_set_filter_result(Gosu_TextInput* text_input, const char* result)
{
    Gosu_translate_exceptions([=] {
        text_input->filter_result = result;
    });
}

GOSU_FFI_API void Gosu_TextInput_insert_text(Gosu_TextInput* text_input, const char* text)
{
    Gosu_translate_exceptions([=] {
        text_input->insert_text(text);
    });
}

GOSU_FFI_API void Gosu_TextInput_delete_backward(Gosu_TextInput* text_input)
{
    Gosu_translate_exceptions([=] {
        text_input->delete_backward();
    });
}

GOSU_FFI_API void Gosu_TextInput_delete_forward(Gosu_TextInput* text_input)
{
    Gosu_translate_exceptions([=] {
        text_input->delete_forward();
    });
}
