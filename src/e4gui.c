#include "e4gui.h"
#include "e4core.h"
#include "e4draw.h"
#include <stdlib.h>
#include <string.h>

// Useful macros
#define MAKE_OBJECT(type)                              \
    GuiObjectT* object = malloc(sizeof(GuiObjectT));   \
    object->focused = false;                           \
    object->state = malloc(sizeof(Gui##type##StateT)); \
    Gui##type##StateT* state = (Gui##type##StateT*)object->state;

#define SET_CALLBACKS(name)             \
    object->_update = _##name##_update; \
    object->_draw = _##name##_draw

#define GET_STATE(type) \
    Gui##type##StateT* state = (Gui##type##StateT*)object->state;

// Gui container setup
GuiContainerT* e4gui_create()
{
    GuiContainerT* container = malloc(sizeof(GuiContainerT));
    container->objects = e4list_create();
    return container;
}

void e4gui_add_object(GuiContainerT* container, GuiObjectT* object)
{
    object->container = container;
    e4list_append(container->objects, object);
}

void e4gui_clean(GuiContainerT* container)
{
    e4list_free(container->objects);
    free(container->objects);
}

void e4gui_update(GuiContainerT* container)
{
    for (i32 i = 0; i < container->objects->len; i++)
    {
        GuiObjectT* object = container->objects->data[i];
        object->_update(object);
    }
}

void e4gui_draw(GuiContainerT* container)
{
    for (i32 i = 0; i < container->objects->len; i++)
    {
        GuiObjectT* object = container->objects->data[i];
        object->_draw(object);
    }
}

// Gui objects
static void _button_update(GuiObjectT* object)
{
}

static void _button_draw(GuiObjectT* object)
{
    GET_STATE(Button);

    e4draw_text(state->label, object->container->theme.button_normal, object->x, object->y);
}

GuiObjectT* e4gui_button(char* label, ButtonCallbackPtr callback, u16 x, u16 y)
{
    MAKE_OBJECT(Button);
    SET_CALLBACKS(button);

    object->x = x;
    object->y = y;
    object->w = strlen(label);
    object->h = 1;

    state->label = label;
    state->callback = callback;

    return object;
}
