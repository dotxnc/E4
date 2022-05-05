#ifndef E4CORE_H
#define E4CORE_H

#include "util.h"

void e4core_loadgl();
void e4core_init(u16 term_width, u16 term_height, u16 char_width, u16 char_height);
void e4core_clean();
void e4core_render();
void e4core_putc(u8 ch, u8 color, u16 x, u16 y);
void e4core_clear(u8 ch, u8 color);

#endif
