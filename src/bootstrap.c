#include "bootstrap.h"
#include "e4core.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>

static GLFWwindow* _window = NULL;
static BootstrapWindowSettingsT _window_settings;
static BootstrapEngineSettingsT _engine_settings;
static bool _running = true;
static f32 _timer = 0.;

static void _bootstrap_key(GLFWwindow* window, i32 key, i32 scan, i32 action, i32 mods)
{
    _window_settings.callbacks.key(key, action, mods);
}

static void _bootstrap_click(GLFWwindow* window, i32 button, i32 action, i32 mods)
{
    f64 x, y;
    glfwGetCursorPos(_window, &x, &y);
    _window_settings.callbacks.click(x, y, action, button);
}

static void _bootstrap_char() {}

static void _bootstrap_cursor(GLFWwindow* window, f64 x, f64 y)
{
    e4core_cursor(x, y);
}

void bootstrap_init(BootstrapWindowSettingsT window_settings, BootstrapEngineSettingsT engine_settings)
{
    if (!glfwInit())
        return;

    glfwWindowHint(GLFW_RESIZABLE, false);
    _window = glfwCreateWindow(window_settings.width,
                               window_settings.height,
                               window_settings.title, NULL, NULL);
    if (!_window)
    {
        glfwTerminate();
        return;
    }
    _window_settings = window_settings;
    _engine_settings = engine_settings;

    glfwSetMouseButtonCallback(_window, _bootstrap_click);
    glfwSetKeyCallback(_window, _bootstrap_key);
    glfwSetCursorPosCallback(_window, _bootstrap_cursor);
    glfwMakeContextCurrent(_window);

    e4core_loadgl();
    e4core_init(engine_settings.term_width,
                engine_settings.term_height,
                engine_settings.char_width,
                engine_settings.char_height,
                window_settings.width,
                window_settings.height);

    _window_settings.callbacks.init();
}

void bootstrap_run()
{

    f32 time_last = glfwGetTime();
    f32 time_delta = 0;
    f32 time_now = 0;
    _timer = time_last;

    while (!glfwWindowShouldClose(_window))
    {
        time_now = glfwGetTime();
        time_delta = time_now - time_last;
        time_last = time_now;
        _timer = time_last;

        glfwPollEvents();

        _window_settings.callbacks.update(time_delta);
        _window_settings.callbacks.draw(time_delta);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        e4core_render();
        glfwSwapBuffers(_window);
    }

    e4core_clean();
    glfwDestroyWindow(_window);
}

void bootstrap_stop()
{
    glfwSetWindowShouldClose(_window, true);
}

f32 bootstrap_time()
{
    return _timer;
}
