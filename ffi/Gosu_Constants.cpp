#pragma once

#include "Gosu_FFI.h"
#include <Gosu/Buttons.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <Gosu/Version.hpp>

#ifdef __cplusplus
extern "C" {
#endif

GOSU_FFI_API const char *Gosu_version()
{
    return Gosu::VERSION.c_str();
}

GOSU_FFI_API const char *Gosu_licenses()
{
    return Gosu::LICENSES.c_str();
}

GOSU_FFI_API unsigned Gosu_MAJOR_VERSION = GOSU_MAJOR_VERSION;
GOSU_FFI_API unsigned Gosu_MINOR_VERSION = GOSU_MINOR_VERSION;
GOSU_FFI_API unsigned Gosu_POINT_VERSION = GOSU_POINT_VERSION;

// Alpha/Blend Modes
GOSU_FFI_API unsigned Gosu_AM_DEFAULT = Gosu::AM_DEFAULT;
GOSU_FFI_API unsigned Gosu_AM_INTERPOLATE = Gosu::AM_INTERPOLATE;
GOSU_FFI_API unsigned Gosu_AM_ADD = Gosu::AM_ADD;
GOSU_FFI_API unsigned Gosu_AM_MULTIPLY = Gosu::AM_MULTIPLY;

// Font Flags
GOSU_FFI_API unsigned Gosu_FF_BOLD = Gosu::FF_BOLD;
GOSU_FFI_API unsigned Gosu_FF_ITALIC = Gosu::FF_ITALIC;
GOSU_FFI_API unsigned Gosu_FF_UNDERLINE = Gosu::FF_UNDERLINE;
GOSU_FFI_API unsigned Gosu_FF_COMBINATIONS = Gosu::FF_COMBINATIONS;

// Alignment
GOSU_FFI_API unsigned Gosu_AL_LEFT = Gosu::AL_LEFT;
GOSU_FFI_API unsigned Gosu_AL_RIGHT = Gosu::AL_RIGHT;
GOSU_FFI_API unsigned Gosu_AL_CENTER = Gosu::AL_CENTER;
GOSU_FFI_API unsigned Gosu_AL_JUSTIFY = Gosu::AL_JUSTIFY;

// Image Flags
GOSU_FFI_API unsigned Gosu_IF_SMOOTH = Gosu::IF_SMOOTH;
GOSU_FFI_API unsigned Gosu_IF_TILEABLE_LEFT = Gosu::IF_TILEABLE_LEFT;
GOSU_FFI_API unsigned Gosu_IF_TILEABLE_TOP = Gosu::IF_TILEABLE_TOP;
GOSU_FFI_API unsigned Gosu_IF_TILEABLE_RIGHT = Gosu::IF_TILEABLE_RIGHT;
GOSU_FFI_API unsigned Gosu_IF_TILEABLE_BOTTOM = Gosu::IF_TILEABLE_BOTTOM;
GOSU_FFI_API unsigned Gosu_IF_TILEABLE = Gosu::IF_TILEABLE;
GOSU_FFI_API unsigned Gosu_IF_RETRO = Gosu::IF_RETRO;

GOSU_FFI_API unsigned Gosu_KB_ESCAPE = Gosu::KB_ESCAPE;
GOSU_FFI_API unsigned Gosu_KB_F1 = Gosu::KB_F1;
GOSU_FFI_API unsigned Gosu_KB_F2 = Gosu::KB_F2;
GOSU_FFI_API unsigned Gosu_KB_F3 = Gosu::KB_F3;
GOSU_FFI_API unsigned Gosu_KB_F4 = Gosu::KB_F4;
GOSU_FFI_API unsigned Gosu_KB_F5 = Gosu::KB_F5;
GOSU_FFI_API unsigned Gosu_KB_F6 = Gosu::KB_F6;
GOSU_FFI_API unsigned Gosu_KB_F7 = Gosu::KB_F7;
GOSU_FFI_API unsigned Gosu_KB_F8 = Gosu::KB_F8;
GOSU_FFI_API unsigned Gosu_KB_F9 = Gosu::KB_F9;
GOSU_FFI_API unsigned Gosu_KB_F10 = Gosu::KB_F10;
GOSU_FFI_API unsigned Gosu_KB_F11 = Gosu::KB_F11;
GOSU_FFI_API unsigned Gosu_KB_F12 = Gosu::KB_F12;
GOSU_FFI_API unsigned Gosu_KB_0 = Gosu::KB_0;
GOSU_FFI_API unsigned Gosu_KB_1 = Gosu::KB_1;
GOSU_FFI_API unsigned Gosu_KB_2 = Gosu::KB_2;
GOSU_FFI_API unsigned Gosu_KB_3 = Gosu::KB_3;
GOSU_FFI_API unsigned Gosu_KB_4 = Gosu::KB_4;
GOSU_FFI_API unsigned Gosu_KB_5 = Gosu::KB_5;
GOSU_FFI_API unsigned Gosu_KB_6 = Gosu::KB_6;
GOSU_FFI_API unsigned Gosu_KB_7 = Gosu::KB_7;
GOSU_FFI_API unsigned Gosu_KB_8 = Gosu::KB_8;
GOSU_FFI_API unsigned Gosu_KB_9 = Gosu::KB_9;
GOSU_FFI_API unsigned Gosu_KB_TAB = Gosu::KB_TAB;
GOSU_FFI_API unsigned Gosu_KB_RETURN = Gosu::KB_RETURN;
GOSU_FFI_API unsigned Gosu_KB_SPACE = Gosu::KB_SPACE;
GOSU_FFI_API unsigned Gosu_KB_LEFT_SHIFT = Gosu::KB_LEFT_SHIFT;
GOSU_FFI_API unsigned Gosu_KB_RIGHT_SHIFT = Gosu::KB_RIGHT_SHIFT;
GOSU_FFI_API unsigned Gosu_KB_LEFT_CONTROL = Gosu::KB_LEFT_CONTROL;
GOSU_FFI_API unsigned Gosu_KB_RIGHT_CONTROL = Gosu::KB_RIGHT_CONTROL;
GOSU_FFI_API unsigned Gosu_KB_LEFT_ALT = Gosu::KB_LEFT_ALT;
GOSU_FFI_API unsigned Gosu_KB_RIGHT_ALT = Gosu::KB_RIGHT_ALT;
GOSU_FFI_API unsigned Gosu_KB_LEFT_META = Gosu::KB_LEFT_META;
GOSU_FFI_API unsigned Gosu_KB_RIGHT_META = Gosu::KB_RIGHT_META;
GOSU_FFI_API unsigned Gosu_KB_BACKSPACE = Gosu::KB_BACKSPACE;
GOSU_FFI_API unsigned Gosu_KB_LEFT = Gosu::KB_LEFT;
GOSU_FFI_API unsigned Gosu_KB_RIGHT = Gosu::KB_RIGHT;
GOSU_FFI_API unsigned Gosu_KB_UP = Gosu::KB_UP;
GOSU_FFI_API unsigned Gosu_KB_DOWN = Gosu::KB_DOWN;
GOSU_FFI_API unsigned Gosu_KB_HOME = Gosu::KB_HOME;
GOSU_FFI_API unsigned Gosu_KB_END = Gosu::KB_END;
GOSU_FFI_API unsigned Gosu_KB_PRINT_SCREEN = Gosu::KB_PRINT_SCREEN;
GOSU_FFI_API unsigned Gosu_KB_SCROLL_LOCK = Gosu::KB_SCROLL_LOCK;
GOSU_FFI_API unsigned Gosu_KB_PAUSE = Gosu::KB_PAUSE;
GOSU_FFI_API unsigned Gosu_KB_INSERT = Gosu::KB_INSERT;
GOSU_FFI_API unsigned Gosu_KB_DELETE = Gosu::KB_DELETE;
GOSU_FFI_API unsigned Gosu_KB_PAGE_UP = Gosu::KB_PAGE_UP;
GOSU_FFI_API unsigned Gosu_KB_PAGE_DOWN = Gosu::KB_PAGE_DOWN;
GOSU_FFI_API unsigned Gosu_KB_ENTER = Gosu::KB_ENTER;
GOSU_FFI_API unsigned Gosu_KB_BACKTICK = Gosu::KB_BACKTICK;
GOSU_FFI_API unsigned Gosu_KB_MINUS = Gosu::KB_MINUS;
GOSU_FFI_API unsigned Gosu_KB_EQUALS = Gosu::KB_EQUALS;
GOSU_FFI_API unsigned Gosu_KB_LEFT_BRACKET = Gosu::KB_LEFT_BRACKET;
GOSU_FFI_API unsigned Gosu_KB_RIGHT_BRACKET = Gosu::KB_RIGHT_BRACKET;
GOSU_FFI_API unsigned Gosu_KB_BACKSLASH = Gosu::KB_BACKSLASH;
GOSU_FFI_API unsigned Gosu_KB_SEMICOLON = Gosu::KB_SEMICOLON;
GOSU_FFI_API unsigned Gosu_KB_APOSTROPHE = Gosu::KB_APOSTROPHE;
GOSU_FFI_API unsigned Gosu_KB_COMMA = Gosu::KB_COMMA;
GOSU_FFI_API unsigned Gosu_KB_PERIOD = Gosu::KB_PERIOD;
GOSU_FFI_API unsigned Gosu_KB_SLASH = Gosu::KB_SLASH;
GOSU_FFI_API unsigned Gosu_KB_CAPS_LOCK = Gosu::KB_CAPS_LOCK;
GOSU_FFI_API unsigned Gosu_KB_A = Gosu::KB_A;
GOSU_FFI_API unsigned Gosu_KB_B = Gosu::KB_B;
GOSU_FFI_API unsigned Gosu_KB_C = Gosu::KB_C;
GOSU_FFI_API unsigned Gosu_KB_D = Gosu::KB_D;
GOSU_FFI_API unsigned Gosu_KB_E = Gosu::KB_E;
GOSU_FFI_API unsigned Gosu_KB_F = Gosu::KB_F;
GOSU_FFI_API unsigned Gosu_KB_G = Gosu::KB_G;
GOSU_FFI_API unsigned Gosu_KB_H = Gosu::KB_H;
GOSU_FFI_API unsigned Gosu_KB_I = Gosu::KB_I;
GOSU_FFI_API unsigned Gosu_KB_J = Gosu::KB_J;
GOSU_FFI_API unsigned Gosu_KB_K = Gosu::KB_K;
GOSU_FFI_API unsigned Gosu_KB_L = Gosu::KB_L;
GOSU_FFI_API unsigned Gosu_KB_M = Gosu::KB_M;
GOSU_FFI_API unsigned Gosu_KB_N = Gosu::KB_N;
GOSU_FFI_API unsigned Gosu_KB_O = Gosu::KB_O;
GOSU_FFI_API unsigned Gosu_KB_P = Gosu::KB_P;
GOSU_FFI_API unsigned Gosu_KB_Q = Gosu::KB_Q;
GOSU_FFI_API unsigned Gosu_KB_R = Gosu::KB_R;
GOSU_FFI_API unsigned Gosu_KB_S = Gosu::KB_S;
GOSU_FFI_API unsigned Gosu_KB_T = Gosu::KB_T;
GOSU_FFI_API unsigned Gosu_KB_U = Gosu::KB_U;
GOSU_FFI_API unsigned Gosu_KB_V = Gosu::KB_V;
GOSU_FFI_API unsigned Gosu_KB_W = Gosu::KB_W;
GOSU_FFI_API unsigned Gosu_KB_X = Gosu::KB_X;
GOSU_FFI_API unsigned Gosu_KB_Y = Gosu::KB_Y;
GOSU_FFI_API unsigned Gosu_KB_Z = Gosu::KB_Z;
GOSU_FFI_API unsigned Gosu_KB_ISO = Gosu::KB_ISO;
GOSU_FFI_API unsigned Gosu_KB_NUMPAD_0 = Gosu::KB_NUMPAD_0;
GOSU_FFI_API unsigned Gosu_KB_NUMPAD_1 = Gosu::KB_NUMPAD_1;
GOSU_FFI_API unsigned Gosu_KB_NUMPAD_2 = Gosu::KB_NUMPAD_2;
GOSU_FFI_API unsigned Gosu_KB_NUMPAD_3 = Gosu::KB_NUMPAD_3;
GOSU_FFI_API unsigned Gosu_KB_NUMPAD_4 = Gosu::KB_NUMPAD_4;
GOSU_FFI_API unsigned Gosu_KB_NUMPAD_5 = Gosu::KB_NUMPAD_5;
GOSU_FFI_API unsigned Gosu_KB_NUMPAD_6 = Gosu::KB_NUMPAD_6;
GOSU_FFI_API unsigned Gosu_KB_NUMPAD_7 = Gosu::KB_NUMPAD_7;
GOSU_FFI_API unsigned Gosu_KB_NUMPAD_8 = Gosu::KB_NUMPAD_8;
GOSU_FFI_API unsigned Gosu_KB_NUMPAD_9 = Gosu::KB_NUMPAD_9;
GOSU_FFI_API unsigned Gosu_KB_NUMPAD_DELETE = Gosu::KB_NUMPAD_DELETE;
GOSU_FFI_API unsigned Gosu_KB_NUMPAD_PLUS = Gosu::KB_NUMPAD_PLUS;
GOSU_FFI_API unsigned Gosu_KB_NUMPAD_MINUS = Gosu::KB_NUMPAD_MINUS;
GOSU_FFI_API unsigned Gosu_KB_NUMPAD_MULTIPLY = Gosu::KB_NUMPAD_MULTIPLY;
GOSU_FFI_API unsigned Gosu_KB_NUMPAD_DIVIDE = Gosu::KB_NUMPAD_DIVIDE;

GOSU_FFI_API unsigned Gosu_MS_LEFT = Gosu::MS_LEFT;
GOSU_FFI_API unsigned Gosu_MS_MIDDLE = Gosu::MS_MIDDLE;
GOSU_FFI_API unsigned Gosu_MS_RIGHT = Gosu::MS_RIGHT;
GOSU_FFI_API unsigned Gosu_MS_WHEEL_UP = Gosu::MS_WHEEL_UP;
GOSU_FFI_API unsigned Gosu_MS_WHEEL_DOWN = Gosu::MS_WHEEL_DOWN;
GOSU_FFI_API unsigned Gosu_MS_OTHER_0 = Gosu::MS_OTHER_0;
GOSU_FFI_API unsigned Gosu_MS_OTHER_1 = Gosu::MS_OTHER_1;
GOSU_FFI_API unsigned Gosu_MS_OTHER_2 = Gosu::MS_OTHER_2;
GOSU_FFI_API unsigned Gosu_MS_OTHER_3 = Gosu::MS_OTHER_3;
GOSU_FFI_API unsigned Gosu_MS_OTHER_4 = Gosu::MS_OTHER_4;
GOSU_FFI_API unsigned Gosu_MS_OTHER_5 = Gosu::MS_OTHER_5;
GOSU_FFI_API unsigned Gosu_MS_OTHER_6 = Gosu::MS_OTHER_6;
GOSU_FFI_API unsigned Gosu_MS_OTHER_7 = Gosu::MS_OTHER_7;

GOSU_FFI_API unsigned Gosu_GP_DPAD_LEFT = Gosu::GP_DPAD_LEFT;
GOSU_FFI_API unsigned Gosu_GP_DPAD_RIGHT = Gosu::GP_DPAD_RIGHT;
GOSU_FFI_API unsigned Gosu_GP_DPAD_UP = Gosu::GP_DPAD_UP;
GOSU_FFI_API unsigned Gosu_GP_DPAD_DOWN = Gosu::GP_DPAD_DOWN;
GOSU_FFI_API unsigned Gosu_GP_BUTTON_0 = Gosu::GP_BUTTON_0;
GOSU_FFI_API unsigned Gosu_GP_BUTTON_1 = Gosu::GP_BUTTON_1;
GOSU_FFI_API unsigned Gosu_GP_BUTTON_2 = Gosu::GP_BUTTON_2;
GOSU_FFI_API unsigned Gosu_GP_BUTTON_3 = Gosu::GP_BUTTON_3;
GOSU_FFI_API unsigned Gosu_GP_BUTTON_4 = Gosu::GP_BUTTON_4;
GOSU_FFI_API unsigned Gosu_GP_BUTTON_5 = Gosu::GP_BUTTON_5;
GOSU_FFI_API unsigned Gosu_GP_BUTTON_6 = Gosu::GP_BUTTON_6;
GOSU_FFI_API unsigned Gosu_GP_BUTTON_7 = Gosu::GP_BUTTON_7;
GOSU_FFI_API unsigned Gosu_GP_BUTTON_8 = Gosu::GP_BUTTON_8;
GOSU_FFI_API unsigned Gosu_GP_BUTTON_9 = Gosu::GP_BUTTON_9;
GOSU_FFI_API unsigned Gosu_GP_BUTTON_10 = Gosu::GP_BUTTON_10;
GOSU_FFI_API unsigned Gosu_GP_BUTTON_11 = Gosu::GP_BUTTON_11;
GOSU_FFI_API unsigned Gosu_GP_BUTTON_12 = Gosu::GP_BUTTON_12;
GOSU_FFI_API unsigned Gosu_GP_BUTTON_13 = Gosu::GP_BUTTON_13;
GOSU_FFI_API unsigned Gosu_GP_BUTTON_14 = Gosu::GP_BUTTON_14;
GOSU_FFI_API unsigned Gosu_GP_BUTTON_15 = Gosu::GP_BUTTON_15;

GOSU_FFI_API unsigned Gosu_GP_0_DPAD_LEFT = Gosu::GP_0_DPAD_LEFT;
GOSU_FFI_API unsigned Gosu_GP_0_DPAD_RIGHT = Gosu::GP_0_DPAD_RIGHT;
GOSU_FFI_API unsigned Gosu_GP_0_DPAD_UP = Gosu::GP_0_DPAD_UP;
GOSU_FFI_API unsigned Gosu_GP_0_DPAD_DOWN = Gosu::GP_0_DPAD_DOWN;
GOSU_FFI_API unsigned Gosu_GP_0_BUTTON_0 = Gosu::GP_0_BUTTON_0;
GOSU_FFI_API unsigned Gosu_GP_0_BUTTON_1 = Gosu::GP_0_BUTTON_1;
GOSU_FFI_API unsigned Gosu_GP_0_BUTTON_2 = Gosu::GP_0_BUTTON_2;
GOSU_FFI_API unsigned Gosu_GP_0_BUTTON_3 = Gosu::GP_0_BUTTON_3;
GOSU_FFI_API unsigned Gosu_GP_0_BUTTON_4 = Gosu::GP_0_BUTTON_4;
GOSU_FFI_API unsigned Gosu_GP_0_BUTTON_5 = Gosu::GP_0_BUTTON_5;
GOSU_FFI_API unsigned Gosu_GP_0_BUTTON_6 = Gosu::GP_0_BUTTON_6;
GOSU_FFI_API unsigned Gosu_GP_0_BUTTON_7 = Gosu::GP_0_BUTTON_7;
GOSU_FFI_API unsigned Gosu_GP_0_BUTTON_8 = Gosu::GP_0_BUTTON_8;
GOSU_FFI_API unsigned Gosu_GP_0_BUTTON_9 = Gosu::GP_0_BUTTON_9;
GOSU_FFI_API unsigned Gosu_GP_0_BUTTON_10 = Gosu::GP_0_BUTTON_10;
GOSU_FFI_API unsigned Gosu_GP_0_BUTTON_11 = Gosu::GP_0_BUTTON_11;
GOSU_FFI_API unsigned Gosu_GP_0_BUTTON_12 = Gosu::GP_0_BUTTON_12;
GOSU_FFI_API unsigned Gosu_GP_0_BUTTON_13 = Gosu::GP_0_BUTTON_13;
GOSU_FFI_API unsigned Gosu_GP_0_BUTTON_14 = Gosu::GP_0_BUTTON_14;
GOSU_FFI_API unsigned Gosu_GP_0_BUTTON_15 = Gosu::GP_0_BUTTON_15;

GOSU_FFI_API unsigned Gosu_GP_1_DPAD_LEFT = Gosu::GP_1_DPAD_LEFT;
GOSU_FFI_API unsigned Gosu_GP_1_DPAD_RIGHT = Gosu::GP_1_DPAD_RIGHT;
GOSU_FFI_API unsigned Gosu_GP_1_DPAD_UP = Gosu::GP_1_DPAD_UP;
GOSU_FFI_API unsigned Gosu_GP_1_DPAD_DOWN = Gosu::GP_1_DPAD_DOWN;
GOSU_FFI_API unsigned Gosu_GP_1_BUTTON_0 = Gosu::GP_1_BUTTON_0;
GOSU_FFI_API unsigned Gosu_GP_1_BUTTON_1 = Gosu::GP_1_BUTTON_1;
GOSU_FFI_API unsigned Gosu_GP_1_BUTTON_2 = Gosu::GP_1_BUTTON_2;
GOSU_FFI_API unsigned Gosu_GP_1_BUTTON_3 = Gosu::GP_1_BUTTON_3;
GOSU_FFI_API unsigned Gosu_GP_1_BUTTON_4 = Gosu::GP_1_BUTTON_4;
GOSU_FFI_API unsigned Gosu_GP_1_BUTTON_5 = Gosu::GP_1_BUTTON_5;
GOSU_FFI_API unsigned Gosu_GP_1_BUTTON_6 = Gosu::GP_1_BUTTON_6;
GOSU_FFI_API unsigned Gosu_GP_1_BUTTON_7 = Gosu::GP_1_BUTTON_7;
GOSU_FFI_API unsigned Gosu_GP_1_BUTTON_8 = Gosu::GP_1_BUTTON_8;
GOSU_FFI_API unsigned Gosu_GP_1_BUTTON_9 = Gosu::GP_1_BUTTON_9;
GOSU_FFI_API unsigned Gosu_GP_1_BUTTON_10 = Gosu::GP_1_BUTTON_10;
GOSU_FFI_API unsigned Gosu_GP_1_BUTTON_11 = Gosu::GP_1_BUTTON_11;
GOSU_FFI_API unsigned Gosu_GP_1_BUTTON_12 = Gosu::GP_1_BUTTON_12;
GOSU_FFI_API unsigned Gosu_GP_1_BUTTON_13 = Gosu::GP_1_BUTTON_13;
GOSU_FFI_API unsigned Gosu_GP_1_BUTTON_14 = Gosu::GP_1_BUTTON_14;
GOSU_FFI_API unsigned Gosu_GP_1_BUTTON_15 = Gosu::GP_1_BUTTON_15;

GOSU_FFI_API unsigned Gosu_GP_2_DPAD_LEFT = Gosu::GP_2_DPAD_LEFT;
GOSU_FFI_API unsigned Gosu_GP_2_DPAD_RIGHT = Gosu::GP_2_DPAD_RIGHT;
GOSU_FFI_API unsigned Gosu_GP_2_DPAD_UP = Gosu::GP_2_DPAD_UP;
GOSU_FFI_API unsigned Gosu_GP_2_DPAD_DOWN = Gosu::GP_2_DPAD_DOWN;
GOSU_FFI_API unsigned Gosu_GP_2_BUTTON_0 = Gosu::GP_2_BUTTON_0;
GOSU_FFI_API unsigned Gosu_GP_2_BUTTON_1 = Gosu::GP_2_BUTTON_1;
GOSU_FFI_API unsigned Gosu_GP_2_BUTTON_2 = Gosu::GP_2_BUTTON_2;
GOSU_FFI_API unsigned Gosu_GP_2_BUTTON_3 = Gosu::GP_2_BUTTON_3;
GOSU_FFI_API unsigned Gosu_GP_2_BUTTON_4 = Gosu::GP_2_BUTTON_4;
GOSU_FFI_API unsigned Gosu_GP_2_BUTTON_5 = Gosu::GP_2_BUTTON_5;
GOSU_FFI_API unsigned Gosu_GP_2_BUTTON_6 = Gosu::GP_2_BUTTON_6;
GOSU_FFI_API unsigned Gosu_GP_2_BUTTON_7 = Gosu::GP_2_BUTTON_7;
GOSU_FFI_API unsigned Gosu_GP_2_BUTTON_8 = Gosu::GP_2_BUTTON_8;
GOSU_FFI_API unsigned Gosu_GP_2_BUTTON_9 = Gosu::GP_2_BUTTON_9;
GOSU_FFI_API unsigned Gosu_GP_2_BUTTON_10 = Gosu::GP_2_BUTTON_10;
GOSU_FFI_API unsigned Gosu_GP_2_BUTTON_11 = Gosu::GP_2_BUTTON_11;
GOSU_FFI_API unsigned Gosu_GP_2_BUTTON_12 = Gosu::GP_2_BUTTON_12;
GOSU_FFI_API unsigned Gosu_GP_2_BUTTON_13 = Gosu::GP_2_BUTTON_13;
GOSU_FFI_API unsigned Gosu_GP_2_BUTTON_14 = Gosu::GP_2_BUTTON_14;
GOSU_FFI_API unsigned Gosu_GP_2_BUTTON_15 = Gosu::GP_2_BUTTON_15;

GOSU_FFI_API unsigned Gosu_GP_3_DPAD_LEFT = Gosu::GP_3_DPAD_LEFT;
GOSU_FFI_API unsigned Gosu_GP_3_DPAD_RIGHT = Gosu::GP_3_DPAD_RIGHT;
GOSU_FFI_API unsigned Gosu_GP_3_DPAD_UP = Gosu::GP_3_DPAD_UP;
GOSU_FFI_API unsigned Gosu_GP_3_DPAD_DOWN = Gosu::GP_3_DPAD_DOWN;
GOSU_FFI_API unsigned Gosu_GP_3_BUTTON_0 = Gosu::GP_3_BUTTON_0;
GOSU_FFI_API unsigned Gosu_GP_3_BUTTON_1 = Gosu::GP_3_BUTTON_1;
GOSU_FFI_API unsigned Gosu_GP_3_BUTTON_2 = Gosu::GP_3_BUTTON_2;
GOSU_FFI_API unsigned Gosu_GP_3_BUTTON_3 = Gosu::GP_3_BUTTON_3;
GOSU_FFI_API unsigned Gosu_GP_3_BUTTON_4 = Gosu::GP_3_BUTTON_4;
GOSU_FFI_API unsigned Gosu_GP_3_BUTTON_5 = Gosu::GP_3_BUTTON_5;
GOSU_FFI_API unsigned Gosu_GP_3_BUTTON_6 = Gosu::GP_3_BUTTON_6;
GOSU_FFI_API unsigned Gosu_GP_3_BUTTON_7 = Gosu::GP_3_BUTTON_7;
GOSU_FFI_API unsigned Gosu_GP_3_BUTTON_8 = Gosu::GP_3_BUTTON_8;
GOSU_FFI_API unsigned Gosu_GP_3_BUTTON_9 = Gosu::GP_3_BUTTON_9;
GOSU_FFI_API unsigned Gosu_GP_3_BUTTON_10 = Gosu::GP_3_BUTTON_10;
GOSU_FFI_API unsigned Gosu_GP_3_BUTTON_11 = Gosu::GP_3_BUTTON_11;
GOSU_FFI_API unsigned Gosu_GP_3_BUTTON_12 = Gosu::GP_3_BUTTON_12;
GOSU_FFI_API unsigned Gosu_GP_3_BUTTON_13 = Gosu::GP_3_BUTTON_13;
GOSU_FFI_API unsigned Gosu_GP_3_BUTTON_14 = Gosu::GP_3_BUTTON_14;
GOSU_FFI_API unsigned Gosu_GP_3_BUTTON_15 = Gosu::GP_3_BUTTON_15;

GOSU_FFI_API unsigned Gosu_GP_LEFT = Gosu::GP_LEFT;
GOSU_FFI_API unsigned Gosu_GP_RIGHT = Gosu::GP_RIGHT;
GOSU_FFI_API unsigned Gosu_GP_UP = Gosu::GP_UP;
GOSU_FFI_API unsigned Gosu_GP_DOWN = Gosu::GP_DOWN;

GOSU_FFI_API unsigned Gosu_GP_0_LEFT = Gosu::GP_0_LEFT;
GOSU_FFI_API unsigned Gosu_GP_0_RIGHT = Gosu::GP_0_RIGHT;
GOSU_FFI_API unsigned Gosu_GP_0_UP = Gosu::GP_0_UP;
GOSU_FFI_API unsigned Gosu_GP_0_DOWN = Gosu::GP_0_DOWN;

GOSU_FFI_API unsigned Gosu_GP_1_LEFT = Gosu::GP_1_LEFT;
GOSU_FFI_API unsigned Gosu_GP_1_RIGHT = Gosu::GP_1_RIGHT;
GOSU_FFI_API unsigned Gosu_GP_1_UP = Gosu::GP_1_UP;
GOSU_FFI_API unsigned Gosu_GP_1_DOWN = Gosu::GP_1_DOWN;

GOSU_FFI_API unsigned Gosu_GP_2_LEFT = Gosu::GP_2_LEFT;
GOSU_FFI_API unsigned Gosu_GP_2_RIGHT = Gosu::GP_2_RIGHT;
GOSU_FFI_API unsigned Gosu_GP_2_UP = Gosu::GP_2_UP;
GOSU_FFI_API unsigned Gosu_GP_2_DOWN = Gosu::GP_2_DOWN;

GOSU_FFI_API unsigned Gosu_GP_3_LEFT = Gosu::GP_3_LEFT;
GOSU_FFI_API unsigned Gosu_GP_3_RIGHT = Gosu::GP_3_RIGHT;
GOSU_FFI_API unsigned Gosu_GP_3_UP = Gosu::GP_3_UP;
GOSU_FFI_API unsigned Gosu_GP_3_DOWN = Gosu::GP_3_DOWN;

GOSU_FFI_API unsigned Gosu_GP_LEFT_STICK_X_AXIS = Gosu::GP_LEFT_STICK_X_AXIS;
GOSU_FFI_API unsigned Gosu_GP_LEFT_STICK_Y_AXIS = Gosu::GP_LEFT_STICK_Y_AXIS;
GOSU_FFI_API unsigned Gosu_GP_RIGHT_STICK_X_AXIS = Gosu::GP_RIGHT_STICK_X_AXIS;
GOSU_FFI_API unsigned Gosu_GP_RIGHT_STICK_Y_AXIS = Gosu::GP_RIGHT_STICK_Y_AXIS;
GOSU_FFI_API unsigned Gosu_GP_LEFT_TRIGGER_AXIS = Gosu::GP_LEFT_TRIGGER_AXIS;
GOSU_FFI_API unsigned Gosu_GP_RIGHT_TRIGGER_AXIS = Gosu::GP_RIGHT_TRIGGER_AXIS;

GOSU_FFI_API unsigned Gosu_GP_0_LEFT_STICK_X_AXIS = Gosu::GP_0_LEFT_STICK_X_AXIS;
GOSU_FFI_API unsigned Gosu_GP_0_LEFT_STICK_Y_AXIS = Gosu::GP_0_LEFT_STICK_Y_AXIS;
GOSU_FFI_API unsigned Gosu_GP_0_RIGHT_STICK_X_AXIS = Gosu::GP_0_RIGHT_STICK_X_AXIS;
GOSU_FFI_API unsigned Gosu_GP_0_RIGHT_STICK_Y_AXIS = Gosu::GP_0_RIGHT_STICK_Y_AXIS;
GOSU_FFI_API unsigned Gosu_GP_0_LEFT_TRIGGER_AXIS = Gosu::GP_0_LEFT_TRIGGER_AXIS;
GOSU_FFI_API unsigned Gosu_GP_0_RIGHT_TRIGGER_AXIS = Gosu::GP_0_RIGHT_TRIGGER_AXIS;

GOSU_FFI_API unsigned Gosu_GP_1_LEFT_STICK_X_AXIS = Gosu::GP_1_LEFT_STICK_X_AXIS;
GOSU_FFI_API unsigned Gosu_GP_1_LEFT_STICK_Y_AXIS = Gosu::GP_1_LEFT_STICK_Y_AXIS;
GOSU_FFI_API unsigned Gosu_GP_1_RIGHT_STICK_X_AXIS = Gosu::GP_1_RIGHT_STICK_X_AXIS;
GOSU_FFI_API unsigned Gosu_GP_1_RIGHT_STICK_Y_AXIS = Gosu::GP_1_RIGHT_STICK_Y_AXIS;
GOSU_FFI_API unsigned Gosu_GP_1_LEFT_TRIGGER_AXIS = Gosu::GP_1_LEFT_TRIGGER_AXIS;
GOSU_FFI_API unsigned Gosu_GP_1_RIGHT_TRIGGER_AXIS = Gosu::GP_1_RIGHT_TRIGGER_AXIS;

GOSU_FFI_API unsigned Gosu_GP_2_LEFT_STICK_X_AXIS = Gosu::GP_2_LEFT_STICK_X_AXIS;
GOSU_FFI_API unsigned Gosu_GP_2_LEFT_STICK_Y_AXIS = Gosu::GP_2_LEFT_STICK_Y_AXIS;
GOSU_FFI_API unsigned Gosu_GP_2_RIGHT_STICK_X_AXIS = Gosu::GP_2_RIGHT_STICK_X_AXIS;
GOSU_FFI_API unsigned Gosu_GP_2_RIGHT_STICK_Y_AXIS = Gosu::GP_2_RIGHT_STICK_Y_AXIS;
GOSU_FFI_API unsigned Gosu_GP_2_LEFT_TRIGGER_AXIS = Gosu::GP_2_LEFT_TRIGGER_AXIS;
GOSU_FFI_API unsigned Gosu_GP_2_RIGHT_TRIGGER_AXIS = Gosu::GP_2_RIGHT_TRIGGER_AXIS;

GOSU_FFI_API unsigned Gosu_GP_3_LEFT_STICK_X_AXIS = Gosu::GP_3_LEFT_STICK_X_AXIS;
GOSU_FFI_API unsigned Gosu_GP_3_LEFT_STICK_Y_AXIS = Gosu::GP_3_LEFT_STICK_Y_AXIS;
GOSU_FFI_API unsigned Gosu_GP_3_RIGHT_STICK_X_AXIS = Gosu::GP_3_RIGHT_STICK_X_AXIS;
GOSU_FFI_API unsigned Gosu_GP_3_RIGHT_STICK_Y_AXIS = Gosu::GP_3_RIGHT_STICK_Y_AXIS;
GOSU_FFI_API unsigned Gosu_GP_3_LEFT_TRIGGER_AXIS = Gosu::GP_3_LEFT_TRIGGER_AXIS;
GOSU_FFI_API unsigned Gosu_GP_3_RIGHT_TRIGGER_AXIS = Gosu::GP_3_RIGHT_TRIGGER_AXIS;

GOSU_FFI_API unsigned Gosu_NUM_GAMEPADS = Gosu::NUM_GAMEPADS;
GOSU_FFI_API unsigned Gosu_NO_BUTTON = Gosu::NO_BUTTON;

#ifdef __cplusplus
}
#endif
