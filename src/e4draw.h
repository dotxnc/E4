#ifndef E4DRAW_H
#define E4DRAW_H

#include "e4util.h"

typedef enum
{
    LineStyle_Single,
    LineStyle_Double,
    LineStyle_SingleVertical,
    LineStyle_SingleHorizontal,
} LineStyleE;

void e4draw_text(const char* text, u8 color, u16 x, u16 y);
void e4draw_textn(const char* text, u8 color, u16 x, u16 y, u16 len);
void e4draw_textf(u8 color, u16 x, u16 y, const char* text, ...);
void e4draw_rect(u16 x, u16 y, u16 w, u16 h, u8 color, LineStyleE style);
void e4draw_fill_rect(u8 ch, u8 color, u16 x, u16 y, u16 w, u16 h);
void e4draw_rich_text(const char* text, u16 x, u16 y);

#endif
