#ifndef E4GUI_H
#define E4GUI_H

#include "e4list.h"
#include "e4util.h"
#include "e4vec.h"

// Just have to settle with the fact that the
// gui module relies on the glfw bootstrapper
//
// FIXME: Decouple this.
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

// Creates a moveable window
bool e4gui_window(const char* title, u16* x, u16* y, u16 w, u16 h);
// Panel creates a 2d box to contain the widgets
void e4gui_panel(u16 x, u16 y, u16 w, u16 h);
// a simple text button
bool e4gui_button(const char* text, u16 x, u16 y, i16 hotkey);
// prototype text field
void e4gui_text(char* buffer, u16* cursor, u16 max, u16 x, u16 y, u16 w);

void e4gui_init();
void e4gui_theme(GuiThemeT theme);
void e4gui_enable();
void e4gui_disable();
void e4gui_click(i32 x, i32 y, InputActionE action, MouseButtonE button);
void e4gui_key(i32 key, InputActionE action, InputModE mod);
void e4gui_truepos(u16* x, u16* y);
void e4gui_finish();

#endif
