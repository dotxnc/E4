
// The basics

#include "bootstrap.h"
#include "e4core.h"
#include "e4draw.h"

#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>

void init();
void update(f32 dt);
void draw(f32 dt);
void click(i32 x, i32 y, InputActionE action);
void key(i32 key, InputActionE action, InputModE mod);

// Initialize context and engine

int main(int argc, char** argv)
{
    BootstrapWindowSettingsT window_settings = {
        .title = "Hello, E4",
        .width = 1280,
        .height = 720,
        .callbacks = (BootstrapCallbacksT){
            .init = init,
            .update = update,
            .draw = draw,
            .click = click,
            .key = key,
        }};

    BootstrapEngineSettingsT engine_settings = {
        .term_width = 128,
        .term_height = 72,
        .char_width = 10,
        .char_height = 10,
    };

    bootstrap_init(window_settings, engine_settings);
    bootstrap_run();

    return 0;
}

// Main runtime starts here

void init()
{
}

void update(f32 dt)
{
}

void draw(f32 dt)
{
    e4core_clear(0x00, 0xff);
    e4draw_text("Hello, E4!", 0xf0, 15 + sin(bootstrap_time() * 2) * 8, 1);
}

void click(i32 x, i32 y, InputActionE action)
{
}

void key(i32 key, InputActionE action, InputModE mod)
{
    if (key == GLFW_KEY_F12 || key == GLFW_KEY_ESCAPE)
    {
        bootstrap_stop();
    }
}
