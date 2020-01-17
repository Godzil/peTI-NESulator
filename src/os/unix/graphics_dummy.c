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

#include <sys/time.h>
#include <time.h>


#include <os_dependent.h>

#include <GLFW/glfw3.h>
//#include <OpenGL/glext.h>

#include <palette.h>

typedef struct GLWindow_t GLWindow;

struct KeyArray
{
    uint8_t lastState;
    uint8_t curState;
    uint8_t debounced;
};

struct GLWindow_t
{
    struct KeyArray keyArray[512];
    GLFWwindow *windows;
    uint8_t *videoMemory;
    GLint videoTexture;
    int WIDTH;
    int HEIGHT;
};

#ifndef GL_TEXTURE_RECTANGLE_EXT
#define GL_TEXTURE_RECTANGLE_EXT GL_TEXTURE_RECTANGLE_NV
#endif

void GLWindowInitEx(GLWindow *g, int w, int h)
{
}

void GLWindowInit(GLWindow *g)
{
}

void ShowScreen(GLWindow *g, int w, int h)
{
}

void setupGL(GLWindow *g, int w, int h)
{
}

void restoreGL(GLWindow *g, int w, int h)
{
}

void kbHandler(GLFWwindow *window, int key, int scan, int action, int mod)
{
}

void sizeHandler(GLFWwindow *window, int xs, int ys)
{
}


void initDisplay(GLWindow *g)
{
}

void drawPixel(GLWindow *gw, int x, int y, uint32_t colour)
{
}

void drawLine(GLWindow *g, int x0, int y0, int x1, int y1, uint32_t colour)
{
}

void drawCircle(GLWindow *g, int xc, int yc, int radius, uint32_t colour)
{
}

void drawRect(GLWindow *g, int x0, int y0, int w, int h, uint32_t colour)
{
}

void drawFillrect(GLWindow *g, int x0, int y0, int w, int h, uint32_t colour)
{
}

void clearScreen(GLWindow *g)
{
}

void updateScreen(GLWindow *g)
{
}

void updateScreenAndWait(GLWindow *g)
{
}

int graphics_init()
{
    return 0;
}

int graphics_drawpixel(long x, long y, long color)
{
    return 0;
}

int graphics_drawline(uint32_t x, uint32_t y, uint32_t x1, uint32_t y1, uint32_t colour)
{
    return 0;
}

int graphics_drawRect(uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint32_t colour)
{
    return 0;
}

int graphics_drawFillrect(int x0, int y0, int w, int h, uint32_t colour)
{
    return 0;
}

int graphics_getScreenSize(int *w, int *h)
{
    *w = 640;
    *h = 320;
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
#if 0
    /* For now don't do anything there */
    long WaitTime;
    static long delta = 0;

    /* Try to sync at 60FPS */
    /* Get current time in microseconds */
    gettimeofday(&timeEnd, NULL);

    WaitTime = (timeEnd.tv_sec) - (timeStart.tv_sec);
    WaitTime *= 1000000;
    WaitTime += (timeEnd.tv_usec - timeStart.tv_usec);

#if !ISPAL && ISNTSC
    /* Calculate the waiting time, 16666 is the time of one frame in microseconds at a 60Hz rate) */
    WaitTime = 16666 - WaitTime + delta;
#elif ISPAL && !ISNTSC
    WaitTime = 20000 - WaitTime + delta;
#endif

#ifndef RUN_COVERAGE
    if ((WaitTime >= 0) && (WaitTime < 100000))
    {
        usleep(WaitTime);
    }
#endif

    /* Now get the time after sleep */
    gettimeofday(&timeStart, NULL);

    /* Now calculate How many microseconds we really spend in sleep and
     calculate a delta for next iteration */
    delta = (timeStart.tv_sec) - (timeEnd.tv_sec);
    delta *= 1000000;
    delta += (timeStart.tv_usec - timeEnd.tv_usec);
    delta = WaitTime - delta;

    console_printf(Console_Default, "Delta:%d\n", delta);

    /* To avoid strange time warp when stopping emulation or using acceleration a lot */
    if ((delta > 10000) || (delta < -10000))
    {
        delta = 0;
    }
#endif
}