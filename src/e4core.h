#ifndef E4CORE_H
#define E4CORE_H

#include "e4util.h"
#include "e4vec.h"

// Background, Foreground
#define MAKE_COLOR(b, f) ((u8)((b << 4) | f))

typedef enum CopyModeE
{
    CopyMode_None,
    CopyMode_Static,
    CopyMode_Background,
    CopyMode_Foreground,
    CopyMode_Both,
} CopyModeE;

// Loads opengl functions
void e4core_loadgl();

// Initialize everything needed for the engine
void e4core_init(u16 term_width, u16 term_height, u16 char_width, u16 char_height, u16 screen_width, u16 screen_height);

// Free memory
void e4core_clean();

// Render the complete scene buffer
void e4core_render();

// Core drawing functions
void e4core_putc(u8 ch, u8 color, u16 x, u16 y);
void e4core_clear(u8 ch, u8 color);

// Get variables
u16 e4core_char_width();
u16 e4core_char_height();
u16 e4core_width();
u16 e4core_height();

// Update palette
void e4core_set_palette(PaletteColorT palette[16]);

// Mouse cursor
void e4core_set_cursor(i32 x, i32 y);
UVec16T e4core_cursor();

// Copy mode
void e4core_set_mode(CopyModeE mode);
CopyModeE e4core_mode();
void e4core_push_mode();
void e4core_pop_mode();

// Input events
void e4core_click(u16 click);
void e4core_key();
void e4core_char();

#endif
