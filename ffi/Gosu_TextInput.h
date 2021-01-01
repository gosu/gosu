#pragma once

#include "Gosu_FFI.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Gosu_TextInput Gosu_TextInput;

GOSU_FFI_API Gosu_TextInput* Gosu_TextInput_create();

GOSU_FFI_API const char* Gosu_TextInput_text(Gosu_TextInput* text_input);
GOSU_FFI_API void Gosu_TextInput_set_text(Gosu_TextInput* text_input, const char* text);

GOSU_FFI_API unsigned Gosu_TextInput_caret_pos(Gosu_TextInput* text_input);
GOSU_FFI_API void Gosu_TextInput_set_caret_pos(Gosu_TextInput* text_input, unsigned pos);

GOSU_FFI_API unsigned Gosu_TextInput_selection_start(Gosu_TextInput* text_input);
GOSU_FFI_API void Gosu_TextInput_set_selection_start(Gosu_TextInput* text_input, unsigned pos);

GOSU_FFI_API void Gosu_TextInput_set_filter(Gosu_TextInput *text_input, void function(void* data, const char* text), void* data);
GOSU_FFI_API void Gosu_TextInput_set_filter_result(Gosu_TextInput *text_input, const char* result);
GOSU_FFI_API void Gosu_TextInput_insert_text(Gosu_TextInput *text_input, const char* text);
GOSU_FFI_API void Gosu_TextInput_delete_backward(Gosu_TextInput *text_input);
GOSU_FFI_API void Gosu_TextInput_delete_forward(Gosu_TextInput *text_input);

GOSU_FFI_API void Gosu_TextInput_destroy(Gosu_TextInput* text_input);

#ifdef __cplusplus
}
#endif