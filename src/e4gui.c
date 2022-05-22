#include "e4gui.h"
#include "e4core.h"
#include "e4draw.h"
#include <stdlib.h>
#include <string.h>

static bool _enabled = true;
static GuiThemeT _theme;
static UVec16T _offset;
static UVec16T _border;
static u16 _window_stack = 0;

static struct
{
    bool this_frame;
    bool is_used;
    u16 x;
    u16 y;
    InputActionE action;
    MouseButtonE button;
} _click_info;

static struct
{
    bool event;
    i32 key;
    InputActionE action;
    InputModE mod;
} _key_info;

void e4gui_init()
{
}

void e4gui_theme(GuiThemeT theme)
{
    _theme = theme;
}

void e4gui_enable()
{
    _enabled = true;
}

void e4gui_disable()
{
    _enabled = false;
}

void e4gui_click(i32 x, i32 y, InputActionE action, MouseButtonE button)
{
    _click_info.this_frame = true;
    _click_info.is_used = false;
    _click_info.x = x / e4core_char_width();
    _click_info.y = y / e4core_char_height();
    _click_info.action = action;
    _click_info.button = button;
}

static bool _clicked_box(InputActionE action, MouseButtonE button, u16 x, u16 y, u16 w, u16 h)
{
    UVec16T cursor = e4core_get_cursor();
    if (_click_info.this_frame &&
        !_click_info.is_used &&
        _click_info.button == button &&
        _click_info.action == action &&
        cursor.x >= x && cursor.x < x + w - 1 &&
        cursor.y >= y && cursor.y <= y + h - 1)
    {
        _click_info.is_used = true;
        return true;
    }
    return false;
}

void e4gui_truepos(u16* _x, u16* _y)
{
    i16 x = *_x;
    i16 y = *_y;
    if (x > _border.x)
    {
        x -= _border.x;
        y++;
    }
    if (y > _border.y)
        y -= _border.y;
    if (x < 0)
        x += _border.x;
    if (y < 0)
        y += _border.y;
    *_x = x + _offset.x;
    *_y = y + _offset.y;
}

void e4gui_finish()
{
    _click_info.this_frame = false;
    _window_stack = 0;
}

bool e4gui_window(const char* title, u16* x, u16* y, u16 w, u16 h)
{
    static bool dragging = false;
    static UVec16T origin = {0, 0};
    static IVec16T offset = {0, 0};
    static UVec16T local = {0, 0};
    static u16 captured_window = 0;

    _window_stack++;

    _offset = (UVec16T){*x + 1, *y + 1};
    _border = (UVec16T){w - 2, h - 2};

    u16 mode = e4core_get_mode();
    e4core_copy_mode(CopyMode_Static);

    // high border
    e4draw_rect(*x, *y, w, h, _theme.window_background << 4 | _theme.window_border_high, LineStyle_Single);

    // low borders
    e4draw_fill_rect(0xb3, _theme.window_background << 4 | _theme.window_border_low, *x + w - 1, *y, 1, h);
    e4draw_fill_rect(0xc4, _theme.window_background << 4 | _theme.window_border_low, *x + 1, *y + h - 1, w - 2, 1);
    e4core_putc(0xbf, _theme.window_background << 4 | _theme.window_border_low, *x + w - 1, *y);
    e4core_putc(0xd9, _theme.window_background << 4 | _theme.window_border_low, *x + w - 1, *y + h - 1);

    // main buffer
    e4draw_fill_rect(0x00, _theme.window_background << 4 | _theme.window_background, *x + 1, *y + 1, w - 2, h - 2);

    // window title
    e4draw_text(title, _theme.window_title_background << 4 | _theme.window_title, *x + ((w / 2) - (strlen(title) / 2 + 1)), *y);

    e4core_copy_mode(mode);

    // dragging
    if (_clicked_box(InputAction_Press, MouseButton_Left, *x, *y, w, 1))
    {
        UVec16T mpos = e4core_get_cursor();
        dragging = true;
        origin.x = *x;
        origin.y = *y;
        local.x = mpos.x - origin.x;
        local.y = mpos.y - origin.y;
        captured_window = _window_stack;
    }

    if (dragging && captured_window == _window_stack)
    {
        UVec16T mpos = e4core_get_cursor();
        offset.x = mpos.x - origin.x;
        offset.y = mpos.y - origin.y;

        *x = origin.x + offset.x - local.x;
        *y = origin.y + offset.y - local.y;

        if (_clicked_box(InputAction_Release, MouseButton_Left, 0, 0, e4core_width(), e4core_height()))
            dragging = false;
    }

    return dragging;
}

void e4gui_panel(u16 x, u16 y, u16 w, u16 h)
{
    _offset = (UVec16T){x, y};
    _border = (UVec16T){w, h};
}

bool e4gui_button(const char* text, u16 x, u16 y, i16 hotkey)
{
    e4gui_truepos(&x, &y);

    if (!_enabled)
    {
        e4draw_text(text, _theme.button_disabled, x, y);
        return false;
    }

    UVec16T cursor = e4core_get_cursor();
    bool hover = cursor.x >= x &&
                 cursor.x < x + strlen(text) &&
                 cursor.y == y;
    u8 color = _theme.button_normal;
    if (hover)
    {
        color = _theme.button_hover;
        if (_click_info.button == MouseButton_Left && _click_info.action == InputAction_Press)
            color = _theme.button_clicked;
    }

    e4draw_text(text, color, x, y);
    if (hotkey > -1 && !hover)
        e4core_putc(text[hotkey], _theme.button_hotkey, x, y);
    e4draw_text(text + hotkey + 1, color, x + hotkey + 1, y);

    return _clicked_box(InputAction_Press, MouseButton_Left, x, y, strlen(text), 1);
}

void e4gui_text(char* buffer, u16* cursor, u16 max, u16 x, u16 y, u16 w)
{
    e4gui_truepos(&x, &y);

    e4core_push_mode();
    e4core_copy_mode(CopyMode_Background);

    e4core_putc(0xd6, _theme.text_outer, x, y);
    e4core_putc(0xba, _theme.text_outer, x, y + 1);
    e4core_putc(0xd3, _theme.text_outer, x, y + 2);

    e4core_putc(0xb7, _theme.text_outer, x + w + 1, y);
    e4core_putc(0xba, _theme.text_outer, x + w + 1, y + 1);
    e4core_putc(0xbd, _theme.text_outer, x + w + 1, y + 2);

    if (!_enabled)
        return;

    e4draw_text(buffer, _theme.text_text, x + 1, y + 1);
    e4core_putc(0xdd, _theme.text_cursor, x + 1 + *cursor, y + 1);

    e4core_pop_mode();
}
