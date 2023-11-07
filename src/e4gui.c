#include "e4gui.h"
#include "bootstrap.h"
#include "e4core.h"
#include "e4draw.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

static f32 _blink = 0;

static void _textbox_update(GuiObjectT* object)
{
    GET_STATE(Textbox);
    u8 ch = bootstrap_get_char();
    bool can_write = state->len < state->max_len-1;
    bool is_empty = state->len == 0;
    if (ch > 0 && can_write)
    {
        state->len++;
        memcpy(state->data + (state->cursor + 1), state->data + state->cursor, state->max_len - state->cursor - 1);
        state->data[state->cursor] = ch;
        state->cursor++;
        _blink = 0;
    }
    else if ((bootstrap_key_pressed(InputKey_Backspace) || bootstrap_key_repeat(InputKey_Backspace)) && !is_empty && state->cursor > 0)
    {
        memcpy(state->data + state->cursor-1, state->data + (state->cursor), state->max_len - state->cursor - 1);

        state->cursor--;
        state->len--;
        _blink = 0;
    }
    else if ((bootstrap_key_pressed(InputKey_Left) ||  bootstrap_key_repeat(InputKey_Left)) && state->cursor > 0)
    {
        state->cursor--;
        _blink = 0;
    }

    else if ((bootstrap_key_pressed(InputKey_Right) ||  bootstrap_key_repeat(InputKey_Right)) && state->cursor < state->len)
    {
        state->cursor++;
        _blink = 0;
    }

    _blink += bootstrap_dtime();
        
}

static void _textbox_draw(GuiObjectT* object)
{
    GET_STATE(Textbox);
    e4core_push_mode();

    e4draw_text(state->data, object->container->theme.text_text, object->x, object->y);
    
    if (fmod(_blink, 2.0) < 1)
        e4core_putc(0xdd, object->container->theme.text_cursor, object->x + state->cursor, object->y);

    e4core_pop_mode();
}

GuiObjectT* e4gui_textbox(char* data, u8 max_len, u16 x, u16 y, u16 width)
{
    MAKE_OBJECT(Textbox);
    SET_CALLBACKS(textbox);

    object->x = x;
    object->y = y;
    object->w = width;

    state->data = data;
    state->max_len = max_len;
    state->len = 0;
    state->use_cursor = true;
    state->cursor = 0;

    // data must be empty
    // TODO: support default/ghost text
    memset(data, 0, max_len);

    return object;
}
