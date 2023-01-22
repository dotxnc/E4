
#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

/*
 *  - Window and engine bootstrapper.
 * This does 2 main things:
 *  1. Creates the opengl context and runs the main render loop
 *  2. Loads the E4 engine and renders it

 * Full list of event callbacks:
 * - Init (void)
 * - Update (f32 dt)
 * - Draw (f32 dt)
 * - Click (i32 x, i32 y, InputActionE action)
 * - Key (i32 key, InputActionE action)
 */

#include "e4util.h"
#include <stdbool.h>

typedef enum InputActionE
{
    InputAction_Release = 0,
    InputAction_Press = 1,
    InputAction_Repeat = 2,
} InputActionE;

typedef enum InputModE
{
    InputMod_None = 0,
    InputMod_Shift = 1,
    InputMod_Control = 2,
    InputMod_Alt = 4,
    InputMod_Super = 8,
} InputModE;

typedef enum MouseButtonE
{
    MouseButton_Left = 0,
    MouseButton_Right = 1,
    MouseButton_Middle = 2,
} MouseButtonE;

typedef enum InputKeyE
{
    InputKey_Unknown = -1,
    InputKey_Space = 32,
    InputKey_Apostrophe = 39,
    InputKey_Comma = 44,
    InputKey_Minus = 45,
    InputKey_Period = 46,
    InputKey_Slash = 47,
    InputKey_0 = 48,
    InputKey_1 = 49,
    InputKey_2 = 50,
    InputKey_3 = 51,
    InputKey_4 = 52,
    InputKey_5 = 53,
    InputKey_6 = 54,
    InputKey_7 = 55,
    InputKey_8 = 56,
    InputKey_9 = 57,
    InputKey_Semicolon = 59,
    InputKey_Equal = 61,
    InputKey_A = 65,
    InputKey_B = 66,
    InputKey_C = 67,
    InputKey_D = 68,
    InputKey_E = 69,
    InputKey_F = 70,
    InputKey_G = 71,
    InputKey_H = 72,
    InputKey_I = 73,
    InputKey_J = 74,
    InputKey_K = 75,
    InputKey_L = 76,
    InputKey_M = 77,
    InputKey_N = 78,
    InputKey_O = 79,
    InputKey_P = 80,
    InputKey_Q = 81,
    InputKey_R = 82,
    InputKey_S = 83,
    InputKey_T = 84,
    InputKey_U = 85,
    InputKey_V = 86,
    InputKey_W = 87,
    InputKey_X = 88,
    InputKey_Y = 89,
    InputKey_Z = 90,
    InputKey_LeftBracket = 91,
    InputKey_Backslash = 92,
    InputKey_RightBracket = 93,
    InputKey_Grave = 96,
    InputKey_Escape = 256,
    InputKey_Enter = 257,
    InputKey_Tab = 258,
    InputKey_Backspace = 259,
    InputKey_Insert = 260,
    InputKey_Delete = 261,
    InputKey_Right = 262,
    InputKey_Left = 263,
    InputKey_Down = 264,
    InputKey_Up = 265,
    InputKey_PageUp = 266,
    InputKey_PageDown = 267,
    InputKey_Home = 268,
    InputKey_End = 269,
    InputKey_CapsLock = 280,
    InputKey_ScrollLock = 281,
    InputKey_NumLock = 282,
    InputKey_PrintScreen = 283,
    InputKey_Pause = 284,
    InputKey_F1 = 290,
    InputKey_F2 = 291,
    InputKey_F3 = 292,
    InputKey_F4 = 293,
    InputKey_F5 = 294,
    InputKey_F6 = 295,
    InputKey_F7 = 296,
    InputKey_F8 = 297,
    InputKey_F9 = 298,
    InputKey_F10 = 299,
    InputKey_F11 = 300,
    InputKey_F12 = 301,
    InputKey_F13 = 302,
    InputKey_F14 = 303,
    InputKey_F15 = 304,
    InputKey_F16 = 305,
    InputKey_F17 = 306,
    InputKey_F18 = 307,
    InputKey_F19 = 308,
    InputKey_F20 = 309,
    InputKey_F21 = 310,
    InputKey_F22 = 311,
    InputKey_F23 = 312,
    InputKey_F24 = 313,
    InputKey_F25 = 314,
    InputKey_KP0 = 320,
    InputKey_KP1 = 321,
    InputKey_KP2 = 322,
    InputKey_KP3 = 323,
    InputKey_KP4 = 324,
    InputKey_KP5 = 325,
    InputKey_KP6 = 326,
    InputKey_KP7 = 327,
    InputKey_KP8 = 328,
    InputKey_KP9 = 329,
    InputKey_KP_Decimal = 330,
    InputKey_KP_Divide = 331,
    InputKey_KP_Multiply = 332,
    InputKey_KP_Subtract = 333,
    InputKey_KP_Add = 334,
    InputKey_KP_Enter = 335,
    InputKey_KP_Equal = 336,
    InputKey_LeftShift = 340,
    InputKey_LeftControl = 341,
    InputKey_LeftAlt = 342,
    InputKey_LeftSuper = 343,
    InputKey_RightShift = 344,
    InputKey_RightControl = 345,
    InputKey_RightAlt = 346,
    InputKey_RightSuper = 347,
    InputKey_Menu = 348,
    InputKey_Count,
} InputKeyE;

typedef void (*BootstrapInitPtr)();
typedef void (*BootstrapUpdatePtr)(f32 dt);
typedef void (*BootstrapDrawPtr)(f32 dt);
typedef void (*BootstrapClickPtr)(i32 x, i32 y, InputActionE action, MouseButtonE button);
typedef void (*BootstrapKeyPtr)(i32 key, InputActionE action, InputModE mod);
typedef struct BootstrapCallbacksT
{
    BootstrapInitPtr init;
    BootstrapUpdatePtr update;
    BootstrapDrawPtr draw;
    BootstrapClickPtr click;
    BootstrapKeyPtr key;
} BootstrapCallbacksT;

typedef struct BootstrapWindowSettingsT
{
    char* title;
    i32 width;
    i32 height;
    BootstrapCallbacksT callbacks;
} BootstrapWindowSettingsT;

typedef struct BootstrapEngineSettingsT
{
    u16 term_width;
    u16 term_height;
    u16 char_width;
    u16 char_height;
} BootstrapEngineSettingsT;

// Creates an opengl context
void bootstrap_init(BootstrapWindowSettingsT window_settings,
                    BootstrapEngineSettingsT engine_settings);
// Start the main render & update loop
void bootstrap_run();
// Stop the main loop and close the window
void bootstrap_stop();
// Current time in seconds
f32 bootstrap_time();
// Frame time delta
f32 bootstrap_dtime();
// Input event helpers
bool bootstrap_key_pressed(InputKeyE key);
bool bootstrap_key_released(InputKeyE key);
bool bootstrap_key_repeat(InputKeyE key);
bool bootstrap_key_down(InputKeyE key);
bool bootstrap_mouse_pressed(MouseButtonE button);
bool bootstrap_mouse_released(MouseButtonE button);
bool bootstrap_mouse_down(MouseButtonE button);
char bootstrap_get_char();

#endif
