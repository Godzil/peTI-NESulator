/*
 *  Graphic Manager - The peTI-NESulator Project
 *  os/macos/graphics.c
 *
 *  Created by Manoël Trapier on 08/05/08.
 *  Copyright (c) 2002-2019 986-Studio.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <os_dependent.h>

#include <palette.h>


int graphics_init()
{

    return 0;
}

static uint32_t getColour(long color)
{
    Palette *pal = &basicPalette[color];
    uint8_t r, g, b, a;
    r = pal->r << 2;
    b = pal->b << 2;
    g = pal->g << 2;
    a = 255;//pal->a;
    return (b << 24) | (g << 16) | (r << 8) | a;
}

int graphics_getScreenSize(int *w, int *h)
{

    return 0;
}

int graphics_drawRect(uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint32_t colour)
{

    return getColour(colour);
}

int graphics_drawFillrect(int x0, int y0, int w, int h, uint32_t colour)
{
    return 0;
}

int graphics_drawpixel(long x, long y, long color)
{

    return 0;
}

int graphics_drawCircle(int xc, int yc, int radius, uint32_t colour)
{

    return 0;
}

int graphics_drawline(uint32_t x, uint32_t y, uint32_t x1, uint32_t y1, uint32_t colour)
{

    return 0;
}

int graphics_blit(long x, long y, long w, long h)
{

    return 0;
}

int getKeyStatus(int key)
{
    return 0;
}

/* Sync with 60Hz (or try to) */
void vsync(void)
{

}