#ifndef E4GUI_H
#define E4GUI_H

#include "e4list.h"
#include "e4util.h"
#include "e4vec.h"

#include "bootstrap.h"

typedef struct GuiThemeT
{
    u16 button_normal;
    u16 button_hover;
    u16 button_clicked;
    u16 button_hotkey;
    u16 button_disabled;

    u16 window_border_high;
    u16 window_border_low;
    u16 window_background;
    u16 window_title;
    u16 window_title_background;

    u16 text_outer;
    u16 text_cursor;
    u16 text_text;
    u16 text_default_text;
} GuiThemeT;

typedef struct GuiStateT GuiStateT;
typedef struct GuiContainerT GuiContainerT;
typedef struct GuiObjectT GuiObjectT;

typedef void (*GuiObjectUpdatePtr)(GuiObjectT* object);
typedef void (*GuiObjectDrawPtr)(GuiObjectT* object);
struct GuiObjectT
{
    GuiContainerT* container;
    GuiStateT* state;
    GuiObjectUpdatePtr _update;
    GuiObjectDrawPtr _draw;
    u16 x, y;
    u16 w, h;
    bool focused;
};

struct GuiContainerT
{
    GuiThemeT theme;
    ListT* objects;
};

GuiContainerT* e4gui_create();
void e4gui_add_object(GuiContainerT* container, GuiObjectT* object);
void e4gui_clean(GuiContainerT* container);
void e4gui_update(GuiContainerT* container);
void e4gui_draw(GuiContainerT* container);

// Gui object definitions
typedef void (*ButtonCallbackPtr)(GuiObjectT* object);
typedef struct GuiButtonStateT
{
    char* label;
    ButtonCallbackPtr callback;
} GuiButtonStateT;
GuiObjectT* e4gui_button(char* label, ButtonCallbackPtr callback, u16 x, u16 y);

#endif
