#include <Gosu/Gosu.hpp>

namespace Gosu
{
  class TextInputForWrapper : public Gosu::TextInput
  {
    public:
      TextInputForWrapper();
      std::string filter(std::string text) const override;
      const char* (*filter_callback)(const char*) = nullptr;
  };
}

Gosu::TextInputForWrapper::TextInputForWrapper() : Gosu::TextInput()
{
}
std::string Gosu::TextInputForWrapper::filter(std::string text) const
{
  if (filter_callback != nullptr) {
    const char *string = filter_callback(text.c_str()); // currently returns NULL...
    return text;
  }
  else {
    return text;
  }
}


extern "C" {
  #include <Gosu/TextInput.h>

  Gosu_TextInput* Gosu_TextInput_create()
  {
    return reinterpret_cast<Gosu_TextInput*>( new Gosu::TextInputForWrapper() );
  }

  const char* Gosu_TextInput_text(Gosu_TextInput* text_input)
  {
    return reinterpret_cast<Gosu::TextInputForWrapper*>( text_input )->text().c_str();
  }

  void Gosu_TextInput_set_text(Gosu_TextInput* text_input, const char* text)
  {
    reinterpret_cast<Gosu::TextInputForWrapper*>( text_input )->set_text(text);
  }

  unsigned Gosu_TextInput_caret_pos(Gosu_TextInput* text_input)
  {
    return reinterpret_cast<Gosu::TextInputForWrapper*>( text_input )->caret_pos();
  }

  void Gosu_TextInput_set_caret_pos(Gosu_TextInput* text_input, unsigned pos)
  {
    return reinterpret_cast<Gosu::TextInputForWrapper*>( text_input )->set_caret_pos(pos);
  }

  unsigned Gosu_TextInput_selection_start(Gosu_TextInput* text_input)
  {
    return reinterpret_cast<Gosu::TextInputForWrapper*>( text_input )->selection_start();
  }

  void Gosu_TextInput_set_selection_start(Gosu_TextInput* text_input, unsigned pos)
  {
    return reinterpret_cast<Gosu::TextInputForWrapper *>(text_input)->set_selection_start(pos);
  }

  void Gosu_TextInput_set_filter(Gosu_TextInput *text_input, const char* function(const char*))
  {
    reinterpret_cast<Gosu::TextInputForWrapper*>( text_input )->filter_callback = function;
  }

  void Gosu_TextInput_insert_text(Gosu_TextInput *text_input, const char* text)
  {
    reinterpret_cast<Gosu::TextInputForWrapper*>( text_input )->insert_text(text);
  }

  void Gosu_TextInput_delete_backward(Gosu_TextInput *text_input)
  {
    reinterpret_cast<Gosu::TextInputForWrapper*>( text_input )->delete_backward();
  }

  void Gosu_TextInput_delete_forward(Gosu_TextInput *text_input)
  {
    reinterpret_cast<Gosu::TextInputForWrapper*>( text_input )->delete_forward();
  }

  void Gosu_TextInput_destroy(Gosu_TextInput* text_input)
  {
    delete( reinterpret_cast<Gosu::TextInputForWrapper*>( text_input ) );
  }
}