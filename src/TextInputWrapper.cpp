#include <Gosu/Gosu.hpp>

extern "C" {
  #include <Gosu/TextInput.h>

  Gosu_TextInput* Gosu_TextInput_create()
  {
    return reinterpret_cast<Gosu_TextInput*>( new Gosu::TextInput() );
  }

  const char* Gosu_TextInput_text(Gosu_TextInput* text_input)
  {
    return reinterpret_cast<Gosu::TextInput*>( text_input )->text().c_str();
  }

  unsigned Gosu_TextInput_caret_pos(Gosu_TextInput* text_input)
  {
    return reinterpret_cast<Gosu::TextInput*>( text_input )->caret_pos();
  }

  void Gosu_TextInput_set_caret_pos(Gosu_TextInput* text_input, unsigned pos)
  {
    return reinterpret_cast<Gosu::TextInput*>( text_input )->set_caret_pos(pos);
  }

  unsigned Gosu_TextInput_selection_start(Gosu_TextInput* text_input)
  {
    return reinterpret_cast<Gosu::TextInput*>( text_input )->selection_start();
  }

  void Gosu_TextInput_set_selection_start(Gosu_TextInput* text_input, unsigned pos)
  {
    return reinterpret_cast<Gosu::TextInput*>( text_input )->set_selection_start(pos);
  }

  void Gosu_TextInput_filter(Gosu_TextInput* text_input, void function())
  {
    // TODO: Add 'shadow' Gosu::TextInputWithCallback? for this callback
    // return reinterpret_cast<Gosu::TextInput*>( text_input )->filter(&function);
  }

  void Gosu_TextInput_destroy(Gosu_TextInput* text_input)
  {
    delete( reinterpret_cast<Gosu::TextInput*>( text_input ) );
  }
}