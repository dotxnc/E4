#include "e4scene.h"

static SceneContainerT _current_scene;

void e4scene_change(SceneContainerT scene)
{
    if (!scene.initialized)
    {
        scene.init();
        scene.initialized = true;
    }
    _current_scene = scene;
    // TODO: "enter" and "leave" callbacks
}

SceneContainerT e4scene_current()
{
    return _current_scene;
}

void e4scene_update()
{
    _current_scene.draw();
}

void e4scene_draw()
{
    _current_scene.update();
}
