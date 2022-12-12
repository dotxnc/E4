#ifndef STATE_H
#define STATE_H

#include "e4util.h"
#include <stdbool.h>

typedef void (*StateMethodPtr)();

typedef struct StateContainerT
{
    StateMethodPtr init;
    StateMethodPtr update;
    StateMethodPtr draw;
    u32 uid;
    bool initialized;
} StateContainerT;

#define REGISTER_STATE(name)                \
    void state_##name##_init();             \
    void state_##name##_update();           \
    void state_##name##_draw();             \
    static StateContainerT state_##name = { \
        .init = state_##name##_init,        \
        .update = state_##name##_update,    \
        .draw = state_##name##_draw,        \
        .uid = __COUNTER__,                 \
        .initialized = false,               \
    };

void e4state_change(StateContainerT state);
StateContainerT e4state_current();

void e4state_update();
void e4state_draw();

#endif
