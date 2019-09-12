#include <Gosu/Gosu.hpp>

namespace Gosu
{
  class TextInputForWrapper : public Gosu::TextInput
  {
    public:
      TextInputForWrapper();
      std::string filter(std::string text) const override;
  };
}

Gosu::TextInputForWrapper::TextInputForWrapper() : Gosu::TextInput()
{
}
std::string Gosu::TextInputForWrapper::filter(std::string text) const
{
  return text;
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

  void Gosu_TextInput_filter(Gosu_TextInput *text_input, void function())
  {
    // TODO: Add 'shadow' Gosu::TextInputForWrapperWithCallback? for this callback
    // return reinterpret_cast<Gosu::TextInputForWrapper*>( text_input )->filter(&function);
  }

  void Gosu_TextInput_destroy(Gosu_TextInput* text_input)
  {
    delete( reinterpret_cast<Gosu::TextInputForWrapper*>( text_input ) );
  }
}