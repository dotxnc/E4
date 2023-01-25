#ifndef SCENE_H
#define SCENE_H

#include "e4util.h"
#include <stdbool.h>

typedef void (*StateMethodPtr)();

typedef struct SceneContainerT
{
    StateMethodPtr init;
    StateMethodPtr update;
    StateMethodPtr draw;
    u32 uid;
    bool initialized;
} SceneContainerT;

#define REGISTER_SCENE(name)                \
    void scene_##name##_init();             \
    void scene_##name##_update();           \
    void scene_##name##_draw();             \
    static SceneContainerT scene_##name = { \
        .init = scene_##name##_init,        \
        .update = scene_##name##_update,    \
        .draw = scene_##name##_draw,        \
        .uid = __COUNTER__,                 \
        .initialized = false,               \
    };

void e4scene_change(SceneContainerT scene);
SceneContainerT e4scene_current();

void e4scene_update();
void e4scene_draw();

#endif
