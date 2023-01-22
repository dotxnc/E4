#include "e4draw.h"
#include "e4core.h"

#include <string.h>

void e4draw_text(const char* text, u8 color, u16 x, u16 y)
{
    for (i32 i = 0; i < strlen(text); i++)
        e4core_putc(text[i], color, x + i, y);
}

void e4draw_textn(const char* text, u8 color, u16 x, u16 y, u16 len)
{
    for (i32 i = 0; i < (len < strlen(text) ? len : strlen(text)); i++)
        e4core_putc(text[i], color, x + i, y);
}

void e4draw_rect(u16 x, u16 y, u16 w, u16 h, u8 color, LineStyleE style)
{
    u8* palette = NULL;
    switch (style)
    {
    case LineStyle_Single:
        palette = (u8[6]){0xda, 0xbf, 0xd9, 0xc0, 0xc4, 0xb3};
        break;
    case LineStyle_Double:
        palette = (u8[6]){0xc9, 0xbb, 0xbc, 0xc8, 0xcd, 0xba};
        break;
    case LineStyle_SingleVertical:
        palette = (u8[6]){0xd5, 0xb8, 0xbe, 0xd4, 0xcd, 0xb3};
        break;
    case LineStyle_SingleHorizontal:
        palette = (u8[6]){0xd6, 0xb7, 0xbd, 0xd3, 0xc4, 0xba};
        break;
    default:
        return;
    }

    enum PaletteSelectionE
    {
        TopLeft,
        TopRight,
        BottomRight,
        BottomLeft,

        Horizontal,
        Vertical,
    };

    for (i32 i = x; i < x + w; i++)
    {
        e4core_putc(palette[Horizontal], color, i, y);
        e4core_putc(palette[Horizontal], color, i, y + h - 1);
    }

    for (i32 i = y; i < y + h; i++)
    {
        e4core_putc(palette[Vertical], color, x, i);
        e4core_putc(palette[Vertical], color, x + w - 1, i);
    }

    e4core_putc(palette[TopLeft], color, x, y);
    e4core_putc(palette[TopRight], color, x + w - 1, y);
    e4core_putc(palette[BottomRight], color, x + w - 1, y + h - 1);
    e4core_putc(palette[BottomLeft], color, x, y + h - 1);
}

void e4draw_fill_rect(u8 ch, u8 color, u16 x, u16 y, u16 w, u16 h)
{
    for (i32 xx = x; xx < x + w; xx++)
        for (i32 yy = y; yy < y + h; yy++)
            e4core_putc(ch, color, xx, yy);
}

void e4draw_rich_text(const char* text, u16 x, u16 y)
{
}
