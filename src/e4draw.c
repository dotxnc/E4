#include "e4draw.h"
#include "e4core.h"

#include <string.h>

void e4draw_text(const char* text, u8 color, u16 x, u16 y)
{
    for (i32 i = 0; i < strlen(text); i++)
        e4core_putc(text[i], color, x + i, y);
}
