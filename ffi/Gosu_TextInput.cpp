#include <Gosu/Gosu.hpp>

namespace Gosu
{
class TextInputForWrapper : public Gosu::TextInput
{
public:
    TextInputForWrapper();
    std::string filter(std::string text) const override;
    std::function<void (const char *text)> filter_callback;
    std::string filter_result = "";
};
} // namespace Gosu

Gosu::TextInputForWrapper::TextInputForWrapper() : Gosu::TextInput()
{
}

std::string Gosu::TextInputForWrapper::filter(std::string text) const
{
    if (filter_callback != nullptr) {
        filter_callback(text.c_str());
        return filter_result;
    }
    else {
        return text;
    }
}

extern "C" {
#include "Gosu_TextInput.h"

Gosu_TextInput *Gosu_TextInput_create()
{
    return reinterpret_cast<Gosu_TextInput *>(new Gosu::TextInputForWrapper());
}

const char *Gosu_TextInput_text(Gosu_TextInput *text_input)
{
    thread_local std::string string;
    string = reinterpret_cast<Gosu::TextInputForWrapper *>(text_input)->text();

    return string.c_str();
}

void Gosu_TextInput_set_text(Gosu_TextInput *text_input, const char *text)
{
    reinterpret_cast<Gosu::TextInputForWrapper *>(text_input)->set_text(text);
}

unsigned Gosu_TextInput_caret_pos(Gosu_TextInput *text_input)
{
    return reinterpret_cast<Gosu::TextInputForWrapper *>(text_input)->caret_pos();
}

void Gosu_TextInput_set_caret_pos(Gosu_TextInput *text_input, unsigned pos)
{
    return reinterpret_cast<Gosu::TextInputForWrapper *>(text_input)->set_caret_pos(pos);
}

unsigned Gosu_TextInput_selection_start(Gosu_TextInput *text_input)
{
    return reinterpret_cast<Gosu::TextInputForWrapper *>(text_input)->selection_start();
}

void Gosu_TextInput_set_selection_start(Gosu_TextInput *text_input, unsigned pos)
{
    return reinterpret_cast<Gosu::TextInputForWrapper *>(text_input)->set_selection_start(pos);
}

void Gosu_TextInput_set_filter(Gosu_TextInput *text_input, void function(void *data, const char *text), void *data)
{
    reinterpret_cast<Gosu::TextInputForWrapper *>(text_input)->filter_callback = [=](const char *text) { function(data, text); };
}

void Gosu_TextInput_set_filter_result(Gosu_TextInput *text_input, const char *result)
{
    reinterpret_cast<Gosu::TextInputForWrapper *>(text_input)->filter_result = result;
}

void Gosu_TextInput_insert_text(Gosu_TextInput *text_input, const char *text)
{
    reinterpret_cast<Gosu::TextInputForWrapper *>(text_input)->insert_text(text);
}

void Gosu_TextInput_delete_backward(Gosu_TextInput *text_input)
{
    reinterpret_cast<Gosu::TextInputForWrapper *>(text_input)->delete_backward();
}

void Gosu_TextInput_delete_forward(Gosu_TextInput *text_input)
{
    reinterpret_cast<Gosu::TextInputForWrapper *>(text_input)->delete_forward();
}

void Gosu_TextInput_destroy(Gosu_TextInput *text_input)
{
    delete (reinterpret_cast<Gosu::TextInputForWrapper *>(text_input));
}

}