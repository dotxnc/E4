
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

#endif
