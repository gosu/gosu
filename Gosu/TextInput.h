#pragma once

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct Gosu_TextInput Gosu_TextInput;

Gosu_TextInput* Gosu_TextInput_create();

const char* Gosu_TextInput_text(Gosu_TextInput* text_input);
void Gosu_TextInput_set_text(Gosu_TextInput* text_input, const char* text);

unsigned Gosu_TextInput_caret_pos(Gosu_TextInput* text_input);
void Gosu_TextInput_set_caret_pos(Gosu_TextInput* text_input, unsigned pos);

unsigned Gosu_TextInput_selection_start(Gosu_TextInput* text_input);
void Gosu_TextInput_set_selection_start(Gosu_TextInput* text_input, unsigned pos);

void Gosu_TextInput_filter(Gosu_TextInput *text_input, void function());

void Gosu_TextInput_destroy(Gosu_TextInput* text_input);

#ifdef __cplusplus
  }
#endif