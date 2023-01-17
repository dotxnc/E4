
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
void click(i32 x, i32 y, InputActionE action, MouseButtonE button);
void key(i32 key, InputActionE action, InputModE mod);

// Initialize context and engine

int main(int argc, char** argv)
{
    BootstrapWindowSettingsT window_settings = {
        .title = "Hello, E4",
        .width = 640,
        .height = 480,
        .callbacks = (BootstrapCallbacksT){
            .init = init,
            .update = update,
            .draw = draw,
            .click = click,
            .key = key,
        }};

    BootstrapEngineSettingsT engine_settings = {
        .term_width = 64,
        .term_height = 48,
        .char_width = 10,
        .char_height = 10,
    };

    bootstrap_init(window_settings, engine_settings);
    bootstrap_run();

    return 0;
}

// Main runtime starts here

enum ColorValueE
{
    Color_Red = 0x00,
    Color_LightRed,
    Color_LightBrown,
    Color_Brown,
    Color_Orange,
    Color_Yellow,
    Color_Green,
    Color_LightGreen,
    Color_White,
    Color_LightBlue,
    Color_Blue,
    Color_DarkBlue,
    Color_LightGrey,
    Color_Grey,
    Color_Purple,
    Color_Black = 0xff,
};

void init()
{
}

void update(f32 dt)
{
}

void draw(f32 dt)
{
    e4core_clear(0x00, MAKE_COLOR(Color_Black, Color_Black));
    e4draw_text("Hello, E4!", MAKE_COLOR(Color_Black, Color_Green), 15 + sin(bootstrap_time() * 2) * 8, 1);
}

void click(i32 x, i32 y, InputActionE action, MouseButtonE button)
{
}

void key(i32 key, InputActionE action, InputModE mod)
{
    if (key == GLFW_KEY_F12 || key == GLFW_KEY_ESCAPE)
    {
        bootstrap_stop();
    }
}
