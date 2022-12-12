#include "e4state.h"

static StateContainerT _current_state;

void e4state_change(StateContainerT state)
{
    if (!state.initialized)
    {
        state.init();
        state.initialized = true;
    }
    _current_state = state;
    // TODO: "enter" and "leave" callbacks
}

StateContainerT e4state_current()
{
    return _current_state;
}

void e4state_update()
{
    _current_state.draw();
}

void e4state_draw()
{
    _current_state.update();
}
