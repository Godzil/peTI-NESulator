/*
 *  OS Dependent functions - The peTI-NESulator Project
 *  os_dependent.h
 *
 *  Created by Manoël Trapier on 08/05/08.
 *  Copyright (c) 2002-2019 986-Studio.
 *
 */

#ifndef OS_DEPENDENT_H
#define OS_DEPENDENT_H

#include <stdint.h>
#include "text.h"

/* File related functions */
/* Graphics related functions */
int graphics_init();
int graphics_drawpixel(long x, long y, long color);
int graphics_blit(long x, long y, long w, long h);
int graphics_drawline(uint32_t x, uint32_t y, uint32_t x1, uint32_t y1, uint32_t colour);
int graphics_drawRect(uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint32_t colour);
int graphics_drawFillrect(int x0, int y0, int w, int h, uint32_t colour);
int graphics_getScreenSize(int *w, int *h);

void vsync(void);

typedef struct Palette_t
{
    uint8_t r, g, b, a;
} Palette;

int getKeyStatus(int key);

/* Sound related functions */

/* IO functions */
void *LoadFilePtr(char *filename);

/* Console functions */
typedef enum ConsoleLevel_t
{
    Console_Error = 0,
    Console_Warning,
    Console_Alert,
    Console_Default,
    Console_Verbose,
    Console_Debug,
} ConsoleLevel;

int console_init(ConsoleLevel DefaultLevel);
int console_printf(const ConsoleLevel level, const char *format, ...);
int console_printf_d(const char *format, ...);


#define KEY_ENTER (257)
#define KEY_LEFT (263)
#define KEY_RIGHT (262)
#define KEY_UP (265)
#define KEY_DOWN (264)
//#define KEY_ENTER 13




#endif /* OS_DEPENDENT_H */