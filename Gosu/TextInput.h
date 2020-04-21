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

void Gosu_TextInput_set_filter(Gosu_TextInput *text_input, void function(void* data, const char* text), void* data);
void Gosu_TextInput_set_filter_result(Gosu_TextInput *text_input, const char* result);
void Gosu_TextInput_insert_text(Gosu_TextInput *text_input, const char* text);
void Gosu_TextInput_delete_backward(Gosu_TextInput *text_input);
void Gosu_TextInput_delete_forward(Gosu_TextInput *text_input);

void Gosu_TextInput_destroy(Gosu_TextInput* text_input);

#ifdef __cplusplus
}
#endif