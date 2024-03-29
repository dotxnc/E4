#ifndef E4CORE_H
#define E4CORE_H

#include "e4util.h"
#include "e4vec.h"

// Background, Foreground
#define MAKE_COLOR(b, f) ((u8)((b << 4) | f))

typedef enum TileFilterE
{
    TileFilter_None,
    TileFilter_Static,
    TileFilter_Background,
    TileFilter_Foreground,
    TileFilter_Both,
} TileFilterE;

/* deprecated enum */
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

// Acts kind of like glScissor
void e4core_scissor(u16 x, u16 y, u16 w, u16 h);

// Core drawing functions
void e4core_putc(u8 ch, u8 color, u16 x, u16 y);
void e4core_clear(u8 ch, u8 color);

// Future me: there's a better solution.
//      I'm just being lazy, sorry.
// Get character at position
u8 e4core_getch(u16 x, u16 y);

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
void e4core_set_filter(TileFilterE mode);
TileFilterE e4core_filter();
void e4core_push_filter();
void e4core_pop_filter();

/* deprecated functions */
#define e4core_set_mode(mode) e4core_set_filter(mode)
#define e4core_mode e4core_filter
#define e4core_push_mode e4core_push_filter
#define e4core_pop_mode e4core_pop_filter

// Input events
void e4core_click(u16 click);
void e4core_key();
void e4core_char();

// Picture in picture.
typedef struct PipStageT {
    u32* buffer;
    u16 term_width;
    u16 term_height;
    u16 char_width;
    u16 char_height;
} PipStageT;

void e4pip_stage(PipStageT* stage);

void e4pip_draw();

#endif
