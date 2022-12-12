#include "e4gui.h"
#include "e4core.h"
#include "e4draw.h"
#include <stdlib.h>
#include <string.h>

// Useful macros
#define MAKE_OBJECT(type)                              \
    GuiObjectT* object = malloc(sizeof(GuiObjectT));   \
    object->focused = false;                           \
    object->enabled = true;                            \
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
    GET_STATE(Button);

    UVec16T cursor = e4core_cursor();
    bool hover = cursor.x >= object->x &&
                 cursor.x < object->x + object->w &&
                 cursor.y == object->y;
    if (hover && bootstrap_mouse_pressed(MouseButton_Left) && state->callback != NULL)
        state->callback(object);
}

static void _button_draw(GuiObjectT* object)
{
    GET_STATE(Button);

    e4core_push_mode();

    u8 color = object->container->theme.button_normal;
    UVec16T cursor = e4core_cursor();
    bool hover = cursor.x >= object->x &&
                 cursor.x < object->x + object->w &&
                 cursor.y == object->y;

    if (hover && object->enabled)
    {
        color = object->container->theme.button_hover;
        if (bootstrap_mouse_down(MouseButton_Left))
            color = object->container->theme.button_clicked;
    }

    e4draw_text(state->label, color, object->x, object->y);

    e4core_pop_mode();
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

static void _dropmenu_update(GuiObjectT* object)
{
    GET_STATE(DropMenu);

    UVec16T cursor = e4core_cursor();
    bool hover = cursor.x >= object->x &&
                 cursor.x < object->x + object->w &&
                 cursor.y == object->y;

    if (bootstrap_mouse_pressed(MouseButton_Left) && hover)
    {
        state->extended = !state->extended;
    }
}

static void _dropmenu_draw(GuiObjectT* object)
{
    GET_STATE(DropMenu);
    e4core_push_mode();

    u8 color = object->container->theme.button_normal;
    UVec16T cursor = e4core_cursor();
    bool hover = cursor.x >= object->x &&
                 cursor.x < object->x + object->w &&
                 cursor.y == object->y;

    if (hover && object->enabled)
    {
        color = object->container->theme.button_hover;
        if (bootstrap_mouse_down(MouseButton_Left))
            color = object->container->theme.button_clicked;
    }

    e4draw_text(state->label, color, object->x, object->y);
    if (state->extended)
    {
        for (i32 i = 0; i < state->table_count; i++)
        {
            DropMenuItemT item = state->table[i];

            u8 color = object->container->theme.button_normal;
            UVec16T cursor = e4core_cursor();
            bool hover = cursor.x >= object->x &&
                         cursor.x < object->x + strlen(item.label) &&
                         cursor.y == object->y + 1 + i;

            if (hover && object->enabled)
            {
                color = object->container->theme.button_hover;
                if (bootstrap_mouse_down(MouseButton_Left))
                    color = object->container->theme.button_clicked;
            }

            e4draw_text(item.label, color, object->x, object->y + 1 + i);
        }
    }

    e4core_pop_mode();
}

GuiObjectT* e4gui_dropmenu(char* label, DropMenuItemT* table, u32 table_count, u16 x, u16 y)
{
    MAKE_OBJECT(DropMenu);
    SET_CALLBACKS(dropmenu);

    object->x = x;
    object->y = y;
    object->w = strlen(label);
    object->h = 1;

    state->label = label;
    state->table = table;
    state->table_count = table_count;
    state->extended = false;

    return object;
}
