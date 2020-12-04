#include <Gosu/Version.hpp>
#include <Gosu/Buttons.hpp>
#include <Gosu/GraphicsBase.hpp>

#ifdef __cplusplus
extern "C" {
#endif

const char *Gosu_version()
{
    return Gosu::VERSION.c_str();
}

const char *Gosu_licenses()
{
    return Gosu::LICENSES.c_str();
}

unsigned Gosu_MAJOR_VERSION = GOSU_MAJOR_VERSION;
unsigned Gosu_MINOR_VERSION = GOSU_MINOR_VERSION;
unsigned Gosu_POINT_VERSION = GOSU_POINT_VERSION;

// Alpha/Blend Modes
unsigned Gosu_AM_DEFAULT = Gosu::AM_DEFAULT;
unsigned Gosu_AM_INTERPOLATE = Gosu::AM_INTERPOLATE;
unsigned Gosu_AM_ADD = Gosu::AM_ADD;
unsigned Gosu_AM_MULTIPLY = Gosu::AM_MULTIPLY;

// Font Flags
unsigned Gosu_FF_BOLD = Gosu::FF_BOLD;
unsigned Gosu_FF_ITALIC = Gosu::FF_ITALIC;
unsigned Gosu_FF_UNDERLINE = Gosu::FF_UNDERLINE;
unsigned Gosu_FF_COMBINATIONS = Gosu::FF_COMBINATIONS;

// Alignment
unsigned Gosu_AL_LEFT = Gosu::AL_LEFT;
unsigned Gosu_AL_RIGHT = Gosu::AL_RIGHT;
unsigned Gosu_AL_CENTER = Gosu::AL_CENTER;
unsigned Gosu_AL_JUSTIFY = Gosu::AL_JUSTIFY;

// Image Flags
unsigned Gosu_IF_SMOOTH = Gosu::IF_SMOOTH;
unsigned Gosu_IF_TILEABLE_LEFT = Gosu::IF_TILEABLE_LEFT;
unsigned Gosu_IF_TILEABLE_TOP = Gosu::IF_TILEABLE_TOP;
unsigned Gosu_IF_TILEABLE_RIGHT = Gosu::IF_TILEABLE_RIGHT;
unsigned Gosu_IF_TILEABLE_BOTTOM = Gosu::IF_TILEABLE_BOTTOM;
unsigned Gosu_IF_TILEABLE = Gosu::IF_TILEABLE;
unsigned Gosu_IF_RETRO = Gosu::IF_RETRO;

unsigned Gosu_KB_ESCAPE = Gosu::KB_ESCAPE;
unsigned Gosu_KB_F1 = Gosu::KB_F1;
unsigned Gosu_KB_F2 = Gosu::KB_F2;
unsigned Gosu_KB_F3 = Gosu::KB_F3;
unsigned Gosu_KB_F4 = Gosu::KB_F4;
unsigned Gosu_KB_F5 = Gosu::KB_F5;
unsigned Gosu_KB_F6 = Gosu::KB_F6;
unsigned Gosu_KB_F7 = Gosu::KB_F7;
unsigned Gosu_KB_F8 = Gosu::KB_F8;
unsigned Gosu_KB_F9 = Gosu::KB_F9;
unsigned Gosu_KB_F10 = Gosu::KB_F10;
unsigned Gosu_KB_F11 = Gosu::KB_F11;
unsigned Gosu_KB_F12 = Gosu::KB_F12;
unsigned Gosu_KB_0 = Gosu::KB_0;
unsigned Gosu_KB_1 = Gosu::KB_1;
unsigned Gosu_KB_2 = Gosu::KB_2;
unsigned Gosu_KB_3 = Gosu::KB_3;
unsigned Gosu_KB_4 = Gosu::KB_4;
unsigned Gosu_KB_5 = Gosu::KB_5;
unsigned Gosu_KB_6 = Gosu::KB_6;
unsigned Gosu_KB_7 = Gosu::KB_7;
unsigned Gosu_KB_8 = Gosu::KB_8;
unsigned Gosu_KB_9 = Gosu::KB_9;
unsigned Gosu_KB_TAB = Gosu::KB_TAB;
unsigned Gosu_KB_RETURN = Gosu::KB_RETURN;
unsigned Gosu_KB_SPACE = Gosu::KB_SPACE;
unsigned Gosu_KB_LEFT_SHIFT = Gosu::KB_LEFT_SHIFT;
unsigned Gosu_KB_RIGHT_SHIFT = Gosu::KB_RIGHT_SHIFT;
unsigned Gosu_KB_LEFT_CONTROL = Gosu::KB_LEFT_CONTROL;
unsigned Gosu_KB_RIGHT_CONTROL = Gosu::KB_RIGHT_CONTROL;
unsigned Gosu_KB_LEFT_ALT = Gosu::KB_LEFT_ALT;
unsigned Gosu_KB_RIGHT_ALT = Gosu::KB_RIGHT_ALT;
unsigned Gosu_KB_LEFT_META = Gosu::KB_LEFT_META;
unsigned Gosu_KB_RIGHT_META = Gosu::KB_RIGHT_META;
unsigned Gosu_KB_BACKSPACE = Gosu::KB_BACKSPACE;
unsigned Gosu_KB_LEFT = Gosu::KB_LEFT;
unsigned Gosu_KB_RIGHT = Gosu::KB_RIGHT;
unsigned Gosu_KB_UP = Gosu::KB_UP;
unsigned Gosu_KB_DOWN = Gosu::KB_DOWN;
unsigned Gosu_KB_HOME = Gosu::KB_HOME;
unsigned Gosu_KB_END = Gosu::KB_END;
unsigned Gosu_KB_PRINT_SCREEN = Gosu::KB_PRINT_SCREEN;
unsigned Gosu_KB_SCROLL_LOCK = Gosu::KB_SCROLL_LOCK;
unsigned Gosu_KB_PAUSE = Gosu::KB_PAUSE;
unsigned Gosu_KB_INSERT = Gosu::KB_INSERT;
unsigned Gosu_KB_DELETE = Gosu::KB_DELETE;
unsigned Gosu_KB_PAGE_UP = Gosu::KB_PAGE_UP;
unsigned Gosu_KB_PAGE_DOWN = Gosu::KB_PAGE_DOWN;
unsigned Gosu_KB_ENTER = Gosu::KB_ENTER;
unsigned Gosu_KB_BACKTICK = Gosu::KB_BACKTICK;
unsigned Gosu_KB_MINUS = Gosu::KB_MINUS;
unsigned Gosu_KB_EQUALS = Gosu::KB_EQUALS;
unsigned Gosu_KB_LEFT_BRACKET = Gosu::KB_LEFT_BRACKET;
unsigned Gosu_KB_RIGHT_BRACKET = Gosu::KB_RIGHT_BRACKET;
unsigned Gosu_KB_BACKSLASH = Gosu::KB_BACKSLASH;
unsigned Gosu_KB_SEMICOLON = Gosu::KB_SEMICOLON;
unsigned Gosu_KB_APOSTROPHE = Gosu::KB_APOSTROPHE;
unsigned Gosu_KB_COMMA = Gosu::KB_COMMA;
unsigned Gosu_KB_PERIOD = Gosu::KB_PERIOD;
unsigned Gosu_KB_SLASH = Gosu::KB_SLASH;
unsigned Gosu_KB_CAPS_LOCK = Gosu::KB_CAPS_LOCK;
unsigned Gosu_KB_A = Gosu::KB_A;
unsigned Gosu_KB_B = Gosu::KB_B;
unsigned Gosu_KB_C = Gosu::KB_C;
unsigned Gosu_KB_D = Gosu::KB_D;
unsigned Gosu_KB_E = Gosu::KB_E;
unsigned Gosu_KB_F = Gosu::KB_F;
unsigned Gosu_KB_G = Gosu::KB_G;
unsigned Gosu_KB_H = Gosu::KB_H;
unsigned Gosu_KB_I = Gosu::KB_I;
unsigned Gosu_KB_J = Gosu::KB_J;
unsigned Gosu_KB_K = Gosu::KB_K;
unsigned Gosu_KB_L = Gosu::KB_L;
unsigned Gosu_KB_M = Gosu::KB_M;
unsigned Gosu_KB_N = Gosu::KB_N;
unsigned Gosu_KB_O = Gosu::KB_O;
unsigned Gosu_KB_P = Gosu::KB_P;
unsigned Gosu_KB_Q = Gosu::KB_Q;
unsigned Gosu_KB_R = Gosu::KB_R;
unsigned Gosu_KB_S = Gosu::KB_S;
unsigned Gosu_KB_T = Gosu::KB_T;
unsigned Gosu_KB_U = Gosu::KB_U;
unsigned Gosu_KB_V = Gosu::KB_V;
unsigned Gosu_KB_W = Gosu::KB_W;
unsigned Gosu_KB_X = Gosu::KB_X;
unsigned Gosu_KB_Y = Gosu::KB_Y;
unsigned Gosu_KB_Z = Gosu::KB_Z;
unsigned Gosu_KB_ISO = Gosu::KB_ISO;
unsigned Gosu_KB_NUMPAD_0 = Gosu::KB_NUMPAD_0;
unsigned Gosu_KB_NUMPAD_1 = Gosu::KB_NUMPAD_1;
unsigned Gosu_KB_NUMPAD_2 = Gosu::KB_NUMPAD_2;
unsigned Gosu_KB_NUMPAD_3 = Gosu::KB_NUMPAD_3;
unsigned Gosu_KB_NUMPAD_4 = Gosu::KB_NUMPAD_4;
unsigned Gosu_KB_NUMPAD_5 = Gosu::KB_NUMPAD_5;
unsigned Gosu_KB_NUMPAD_6 = Gosu::KB_NUMPAD_6;
unsigned Gosu_KB_NUMPAD_7 = Gosu::KB_NUMPAD_7;
unsigned Gosu_KB_NUMPAD_8 = Gosu::KB_NUMPAD_8;
unsigned Gosu_KB_NUMPAD_9 = Gosu::KB_NUMPAD_9;
unsigned Gosu_KB_NUMPAD_DELETE = Gosu::KB_NUMPAD_DELETE;
unsigned Gosu_KB_NUMPAD_PLUS = Gosu::KB_NUMPAD_PLUS;
unsigned Gosu_KB_NUMPAD_MINUS = Gosu::KB_NUMPAD_MINUS;
unsigned Gosu_KB_NUMPAD_MULTIPLY = Gosu::KB_NUMPAD_MULTIPLY;
unsigned Gosu_KB_NUMPAD_DIVIDE = Gosu::KB_NUMPAD_DIVIDE;

unsigned Gosu_MS_LEFT = Gosu::MS_LEFT;
unsigned Gosu_MS_MIDDLE = Gosu::MS_MIDDLE;
unsigned Gosu_MS_RIGHT = Gosu::MS_RIGHT;
unsigned Gosu_MS_WHEEL_UP = Gosu::MS_WHEEL_UP;
unsigned Gosu_MS_WHEEL_DOWN = Gosu::MS_WHEEL_DOWN;
unsigned Gosu_MS_OTHER_0 = Gosu::MS_OTHER_0;
unsigned Gosu_MS_OTHER_1 = Gosu::MS_OTHER_1;
unsigned Gosu_MS_OTHER_2 = Gosu::MS_OTHER_2;
unsigned Gosu_MS_OTHER_3 = Gosu::MS_OTHER_3;
unsigned Gosu_MS_OTHER_4 = Gosu::MS_OTHER_4;
unsigned Gosu_MS_OTHER_5 = Gosu::MS_OTHER_5;
unsigned Gosu_MS_OTHER_6 = Gosu::MS_OTHER_6;
unsigned Gosu_MS_OTHER_7 = Gosu::MS_OTHER_7;

unsigned Gosu_GP_DPAD_LEFT = Gosu::GP_DPAD_LEFT;
unsigned Gosu_GP_DPAD_RIGHT = Gosu::GP_DPAD_RIGHT;
unsigned Gosu_GP_DPAD_UP = Gosu::GP_DPAD_UP;
unsigned Gosu_GP_DPAD_DOWN = Gosu::GP_DPAD_DOWN;
unsigned Gosu_GP_BUTTON_0 = Gosu::GP_BUTTON_0;
unsigned Gosu_GP_BUTTON_1 = Gosu::GP_BUTTON_1;
unsigned Gosu_GP_BUTTON_2 = Gosu::GP_BUTTON_2;
unsigned Gosu_GP_BUTTON_3 = Gosu::GP_BUTTON_3;
unsigned Gosu_GP_BUTTON_4 = Gosu::GP_BUTTON_4;
unsigned Gosu_GP_BUTTON_5 = Gosu::GP_BUTTON_5;
unsigned Gosu_GP_BUTTON_6 = Gosu::GP_BUTTON_6;
unsigned Gosu_GP_BUTTON_7 = Gosu::GP_BUTTON_7;
unsigned Gosu_GP_BUTTON_8 = Gosu::GP_BUTTON_8;
unsigned Gosu_GP_BUTTON_9 = Gosu::GP_BUTTON_9;
unsigned Gosu_GP_BUTTON_10 = Gosu::GP_BUTTON_10;
unsigned Gosu_GP_BUTTON_11 = Gosu::GP_BUTTON_11;
unsigned Gosu_GP_BUTTON_12 = Gosu::GP_BUTTON_12;
unsigned Gosu_GP_BUTTON_13 = Gosu::GP_BUTTON_13;
unsigned Gosu_GP_BUTTON_14 = Gosu::GP_BUTTON_14;
unsigned Gosu_GP_BUTTON_15 = Gosu::GP_BUTTON_15;

unsigned Gosu_GP_0_DPAD_LEFT = Gosu::GP_0_DPAD_LEFT;
unsigned Gosu_GP_0_DPAD_RIGHT = Gosu::GP_0_DPAD_RIGHT;
unsigned Gosu_GP_0_DPAD_UP = Gosu::GP_0_DPAD_UP;
unsigned Gosu_GP_0_DPAD_DOWN = Gosu::GP_0_DPAD_DOWN;
unsigned Gosu_GP_0_BUTTON_0 = Gosu::GP_0_BUTTON_0;
unsigned Gosu_GP_0_BUTTON_1 = Gosu::GP_0_BUTTON_1;
unsigned Gosu_GP_0_BUTTON_2 = Gosu::GP_0_BUTTON_2;
unsigned Gosu_GP_0_BUTTON_3 = Gosu::GP_0_BUTTON_3;
unsigned Gosu_GP_0_BUTTON_4 = Gosu::GP_0_BUTTON_4;
unsigned Gosu_GP_0_BUTTON_5 = Gosu::GP_0_BUTTON_5;
unsigned Gosu_GP_0_BUTTON_6 = Gosu::GP_0_BUTTON_6;
unsigned Gosu_GP_0_BUTTON_7 = Gosu::GP_0_BUTTON_7;
unsigned Gosu_GP_0_BUTTON_8 = Gosu::GP_0_BUTTON_8;
unsigned Gosu_GP_0_BUTTON_9 = Gosu::GP_0_BUTTON_9;
unsigned Gosu_GP_0_BUTTON_10 = Gosu::GP_0_BUTTON_10;
unsigned Gosu_GP_0_BUTTON_11 = Gosu::GP_0_BUTTON_11;
unsigned Gosu_GP_0_BUTTON_12 = Gosu::GP_0_BUTTON_12;
unsigned Gosu_GP_0_BUTTON_13 = Gosu::GP_0_BUTTON_13;
unsigned Gosu_GP_0_BUTTON_14 = Gosu::GP_0_BUTTON_14;
unsigned Gosu_GP_0_BUTTON_15 = Gosu::GP_0_BUTTON_15;

unsigned Gosu_GP_1_DPAD_LEFT = Gosu::GP_1_DPAD_LEFT;
unsigned Gosu_GP_1_DPAD_RIGHT = Gosu::GP_1_DPAD_RIGHT;
unsigned Gosu_GP_1_DPAD_UP = Gosu::GP_1_DPAD_UP;
unsigned Gosu_GP_1_DPAD_DOWN = Gosu::GP_1_DPAD_DOWN;
unsigned Gosu_GP_1_BUTTON_0 = Gosu::GP_1_BUTTON_0;
unsigned Gosu_GP_1_BUTTON_1 = Gosu::GP_1_BUTTON_1;
unsigned Gosu_GP_1_BUTTON_2 = Gosu::GP_1_BUTTON_2;
unsigned Gosu_GP_1_BUTTON_3 = Gosu::GP_1_BUTTON_3;
unsigned Gosu_GP_1_BUTTON_4 = Gosu::GP_1_BUTTON_4;
unsigned Gosu_GP_1_BUTTON_5 = Gosu::GP_1_BUTTON_5;
unsigned Gosu_GP_1_BUTTON_6 = Gosu::GP_1_BUTTON_6;
unsigned Gosu_GP_1_BUTTON_7 = Gosu::GP_1_BUTTON_7;
unsigned Gosu_GP_1_BUTTON_8 = Gosu::GP_1_BUTTON_8;
unsigned Gosu_GP_1_BUTTON_9 = Gosu::GP_1_BUTTON_9;
unsigned Gosu_GP_1_BUTTON_10 = Gosu::GP_1_BUTTON_10;
unsigned Gosu_GP_1_BUTTON_11 = Gosu::GP_1_BUTTON_11;
unsigned Gosu_GP_1_BUTTON_12 = Gosu::GP_1_BUTTON_12;
unsigned Gosu_GP_1_BUTTON_13 = Gosu::GP_1_BUTTON_13;
unsigned Gosu_GP_1_BUTTON_14 = Gosu::GP_1_BUTTON_14;
unsigned Gosu_GP_1_BUTTON_15 = Gosu::GP_1_BUTTON_15;

unsigned Gosu_GP_2_DPAD_LEFT = Gosu::GP_2_DPAD_LEFT;
unsigned Gosu_GP_2_DPAD_RIGHT = Gosu::GP_2_DPAD_RIGHT;
unsigned Gosu_GP_2_DPAD_UP = Gosu::GP_2_DPAD_UP;
unsigned Gosu_GP_2_DPAD_DOWN = Gosu::GP_2_DPAD_DOWN;
unsigned Gosu_GP_2_BUTTON_0 = Gosu::GP_2_BUTTON_0;
unsigned Gosu_GP_2_BUTTON_1 = Gosu::GP_2_BUTTON_1;
unsigned Gosu_GP_2_BUTTON_2 = Gosu::GP_2_BUTTON_2;
unsigned Gosu_GP_2_BUTTON_3 = Gosu::GP_2_BUTTON_3;
unsigned Gosu_GP_2_BUTTON_4 = Gosu::GP_2_BUTTON_4;
unsigned Gosu_GP_2_BUTTON_5 = Gosu::GP_2_BUTTON_5;
unsigned Gosu_GP_2_BUTTON_6 = Gosu::GP_2_BUTTON_6;
unsigned Gosu_GP_2_BUTTON_7 = Gosu::GP_2_BUTTON_7;
unsigned Gosu_GP_2_BUTTON_8 = Gosu::GP_2_BUTTON_8;
unsigned Gosu_GP_2_BUTTON_9 = Gosu::GP_2_BUTTON_9;
unsigned Gosu_GP_2_BUTTON_10 = Gosu::GP_2_BUTTON_10;
unsigned Gosu_GP_2_BUTTON_11 = Gosu::GP_2_BUTTON_11;
unsigned Gosu_GP_2_BUTTON_12 = Gosu::GP_2_BUTTON_12;
unsigned Gosu_GP_2_BUTTON_13 = Gosu::GP_2_BUTTON_13;
unsigned Gosu_GP_2_BUTTON_14 = Gosu::GP_2_BUTTON_14;
unsigned Gosu_GP_2_BUTTON_15 = Gosu::GP_2_BUTTON_15;

unsigned Gosu_GP_3_DPAD_LEFT = Gosu::GP_3_DPAD_LEFT;
unsigned Gosu_GP_3_DPAD_RIGHT = Gosu::GP_3_DPAD_RIGHT;
unsigned Gosu_GP_3_DPAD_UP = Gosu::GP_3_DPAD_UP;
unsigned Gosu_GP_3_DPAD_DOWN = Gosu::GP_3_DPAD_DOWN;
unsigned Gosu_GP_3_BUTTON_0 = Gosu::GP_3_BUTTON_0;
unsigned Gosu_GP_3_BUTTON_1 = Gosu::GP_3_BUTTON_1;
unsigned Gosu_GP_3_BUTTON_2 = Gosu::GP_3_BUTTON_2;
unsigned Gosu_GP_3_BUTTON_3 = Gosu::GP_3_BUTTON_3;
unsigned Gosu_GP_3_BUTTON_4 = Gosu::GP_3_BUTTON_4;
unsigned Gosu_GP_3_BUTTON_5 = Gosu::GP_3_BUTTON_5;
unsigned Gosu_GP_3_BUTTON_6 = Gosu::GP_3_BUTTON_6;
unsigned Gosu_GP_3_BUTTON_7 = Gosu::GP_3_BUTTON_7;
unsigned Gosu_GP_3_BUTTON_8 = Gosu::GP_3_BUTTON_8;
unsigned Gosu_GP_3_BUTTON_9 = Gosu::GP_3_BUTTON_9;
unsigned Gosu_GP_3_BUTTON_10 = Gosu::GP_3_BUTTON_10;
unsigned Gosu_GP_3_BUTTON_11 = Gosu::GP_3_BUTTON_11;
unsigned Gosu_GP_3_BUTTON_12 = Gosu::GP_3_BUTTON_12;
unsigned Gosu_GP_3_BUTTON_13 = Gosu::GP_3_BUTTON_13;
unsigned Gosu_GP_3_BUTTON_14 = Gosu::GP_3_BUTTON_14;
unsigned Gosu_GP_3_BUTTON_15 = Gosu::GP_3_BUTTON_15;

unsigned Gosu_GP_LEFT = Gosu::GP_LEFT;
unsigned Gosu_GP_RIGHT = Gosu::GP_RIGHT;
unsigned Gosu_GP_UP = Gosu::GP_UP;
unsigned Gosu_GP_DOWN = Gosu::GP_DOWN;

unsigned Gosu_GP_0_LEFT = Gosu::GP_0_LEFT;
unsigned Gosu_GP_0_RIGHT = Gosu::GP_0_RIGHT;
unsigned Gosu_GP_0_UP = Gosu::GP_0_UP;
unsigned Gosu_GP_0_DOWN = Gosu::GP_0_DOWN;

unsigned Gosu_GP_1_LEFT = Gosu::GP_1_LEFT;
unsigned Gosu_GP_1_RIGHT = Gosu::GP_1_RIGHT;
unsigned Gosu_GP_1_UP = Gosu::GP_1_UP;
unsigned Gosu_GP_1_DOWN = Gosu::GP_1_DOWN;

unsigned Gosu_GP_2_LEFT = Gosu::GP_2_LEFT;
unsigned Gosu_GP_2_RIGHT = Gosu::GP_2_RIGHT;
unsigned Gosu_GP_2_UP = Gosu::GP_2_UP;
unsigned Gosu_GP_2_DOWN = Gosu::GP_2_DOWN;

unsigned Gosu_GP_3_LEFT = Gosu::GP_3_LEFT;
unsigned Gosu_GP_3_RIGHT = Gosu::GP_3_RIGHT;
unsigned Gosu_GP_3_UP = Gosu::GP_3_UP;
unsigned Gosu_GP_3_DOWN = Gosu::GP_3_DOWN;

unsigned Gosu_GP_LEFT_STICK_X_AXIS = Gosu::GP_LEFT_STICK_X_AXIS;
unsigned Gosu_GP_LEFT_STICK_Y_AXIS = Gosu::GP_LEFT_STICK_Y_AXIS;
unsigned Gosu_GP_RIGHT_STICK_X_AXIS = Gosu::GP_RIGHT_STICK_X_AXIS;
unsigned Gosu_GP_RIGHT_STICK_Y_AXIS = Gosu::GP_RIGHT_STICK_Y_AXIS;
unsigned Gosu_GP_LEFT_TRIGGER_AXIS = Gosu::GP_LEFT_TRIGGER_AXIS;
unsigned Gosu_GP_RIGHT_TRIGGER_AXIS = Gosu::GP_RIGHT_TRIGGER_AXIS;

unsigned Gosu_GP_0_LEFT_STICK_X_AXIS = Gosu::GP_0_LEFT_STICK_X_AXIS;
unsigned Gosu_GP_0_LEFT_STICK_Y_AXIS = Gosu::GP_0_LEFT_STICK_Y_AXIS;
unsigned Gosu_GP_0_RIGHT_STICK_X_AXIS = Gosu::GP_0_RIGHT_STICK_X_AXIS;
unsigned Gosu_GP_0_RIGHT_STICK_Y_AXIS = Gosu::GP_0_RIGHT_STICK_Y_AXIS;
unsigned Gosu_GP_0_LEFT_TRIGGER_AXIS = Gosu::GP_0_LEFT_TRIGGER_AXIS;
unsigned Gosu_GP_0_RIGHT_TRIGGER_AXIS = Gosu::GP_0_RIGHT_TRIGGER_AXIS;

unsigned Gosu_GP_1_LEFT_STICK_X_AXIS = Gosu::GP_1_LEFT_STICK_X_AXIS;
unsigned Gosu_GP_1_LEFT_STICK_Y_AXIS = Gosu::GP_1_LEFT_STICK_Y_AXIS;
unsigned Gosu_GP_1_RIGHT_STICK_X_AXIS = Gosu::GP_1_RIGHT_STICK_X_AXIS;
unsigned Gosu_GP_1_RIGHT_STICK_Y_AXIS = Gosu::GP_1_RIGHT_STICK_Y_AXIS;
unsigned Gosu_GP_1_LEFT_TRIGGER_AXIS = Gosu::GP_1_LEFT_TRIGGER_AXIS;
unsigned Gosu_GP_1_RIGHT_TRIGGER_AXIS = Gosu::GP_1_RIGHT_TRIGGER_AXIS;

unsigned Gosu_GP_2_LEFT_STICK_X_AXIS = Gosu::GP_2_LEFT_STICK_X_AXIS;
unsigned Gosu_GP_2_LEFT_STICK_Y_AXIS = Gosu::GP_2_LEFT_STICK_Y_AXIS;
unsigned Gosu_GP_2_RIGHT_STICK_X_AXIS = Gosu::GP_2_RIGHT_STICK_X_AXIS;
unsigned Gosu_GP_2_RIGHT_STICK_Y_AXIS = Gosu::GP_2_RIGHT_STICK_Y_AXIS;
unsigned Gosu_GP_2_LEFT_TRIGGER_AXIS = Gosu::GP_2_LEFT_TRIGGER_AXIS;
unsigned Gosu_GP_2_RIGHT_TRIGGER_AXIS = Gosu::GP_2_RIGHT_TRIGGER_AXIS;

unsigned Gosu_GP_3_LEFT_STICK_X_AXIS = Gosu::GP_3_LEFT_STICK_X_AXIS;
unsigned Gosu_GP_3_LEFT_STICK_Y_AXIS = Gosu::GP_3_LEFT_STICK_Y_AXIS;
unsigned Gosu_GP_3_RIGHT_STICK_X_AXIS = Gosu::GP_3_RIGHT_STICK_X_AXIS;
unsigned Gosu_GP_3_RIGHT_STICK_Y_AXIS = Gosu::GP_3_RIGHT_STICK_Y_AXIS;
unsigned Gosu_GP_3_LEFT_TRIGGER_AXIS = Gosu::GP_3_LEFT_TRIGGER_AXIS;
unsigned Gosu_GP_3_RIGHT_TRIGGER_AXIS = Gosu::GP_3_RIGHT_TRIGGER_AXIS;

unsigned Gosu_NUM_GAMEPADS = Gosu::NUM_GAMEPADS;
unsigned Gosu_NO_BUTTON = Gosu::NO_BUTTON;

#ifdef __cplusplus
}
#endif
