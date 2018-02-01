/*
 *  Graphic Manager - The peTI-NESulator Project
 *  os/macos/graphics.c
 *
 *  Created by Manoel TRAPIER on 08/05/08.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

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
   GLFWwindow* windows;
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

void kbHandler(GLFWwindow* window, int key, int scan, int action, int mod )
{
}

void sizeHandler(GLFWwindow* window,int xs,int ys)
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

int graphics_drawline(long x, long y, long x1, long y1, long color)
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
