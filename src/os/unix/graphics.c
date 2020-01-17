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

#define GLFW_INCLUDE_GLEXT

#include <GLFW/glfw3.h>

/* "Apple" fix */
#ifndef GL_TEXTURE_RECTANGLE
#define GL_TEXTURE_RECTANGLE GL_TEXTURE_RECTANGLE_EXT
#endif

#include <palette.h>

typedef struct GLWindow_t GLWindow;

struct KeyArray
{
    uint8_t lastState;
    uint8_t curState;
    uint8_t debounced;
    GLFWwindow *window;
};

struct GLWindow_t
{
    struct KeyArray keyArray[512];
    GLFWwindow *windows;
    uint8_t *videoMemory;
    GLuint videoTexture;
    int WIDTH;
    int HEIGHT;
};

static int window_num = 0;

void GLWindowInitEx(GLWindow *g, int w, int h)
{
    g->WIDTH = w;
    g->HEIGHT = h;
    g->videoTexture = window_num++;
}

void GLWindowInit(GLWindow *g)
{
    GLWindowInitEx(g, 100, 100);
}

void ShowScreen(GLWindow *g, int w, int h)
{
    glBindTexture(GL_TEXTURE_RECTANGLE, g->videoTexture);

    // glTexSubImage2D is faster when not using a texture range
    glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, w, h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, g->videoMemory);
    glBegin(GL_QUADS);

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0f, 1.0f);

    glTexCoord2f(0.0f, h);
    glVertex2f(-1.0f, -1.0f);

    glTexCoord2f(w, h);
    glVertex2f(1.0f, -1.0f);

    glTexCoord2f(w, 0.0f);
    glVertex2f(1.0f, 1.0f);
    glEnd();

    glFlush();
}

void setupGL(GLWindow *g, int w, int h)
{
    g->videoMemory = (uint8_t *)malloc(w * h * sizeof(uint32_t));
    memset(g->videoMemory, 0, w * h * sizeof(uint32_t));
    //Tell OpenGL how to convert from coordinates to pixel values
    glViewport(0, 0, w, h);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glClearColor(1.0f, 0.f, 1.0f, 1.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_RECTANGLE);
    glBindTexture(GL_TEXTURE_RECTANGLE, g->videoTexture);

    //  glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE_NV_EXT, 0, NULL);

    //  glTexParameteri(GL_TEXTURE_RECTANGLE_NV_EXT, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_CACHED_APPLE);
    //  glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, w,
                 h, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, g->videoMemory);

    glDisable(GL_DEPTH_TEST);
}

void restoreGL(GLWindow *g, int w, int h)
{
    //Tell OpenGL how to convert from coordinates to pixel values
    glViewport(0, 0, w, h);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glClearColor(1.0f, 0.f, 1.0f, 1.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_RECTANGLE);
    glDisable(GL_DEPTH_TEST);
}

void kbHandler(GLFWwindow *window, int key, int scan, int action, int mod)
{
    struct KeyArray *keyArray;

    keyArray = (struct KeyArray *)glfwGetWindowUserPointer(window);

    keyArray[key].lastState = keyArray[key].curState;
    if (action == GLFW_RELEASE)
    {
        keyArray[key].curState = GLFW_RELEASE;
    }
    else
    {
        keyArray[key].curState = GLFW_PRESS;
    }
    keyArray[key].debounced |= (keyArray[key].lastState == GLFW_RELEASE) && (keyArray[key].curState == GLFW_PRESS);
    keyArray[key].window = window;
    /*printf("key:%d, state:%d debounce:%d, laststate:%d\n", key, keyArray[key].curState,
           keyArray[key].debounced, keyArray[key].lastState);*/
}

void sizeHandler(GLFWwindow *window, int xs, int ys)
{
    glfwMakeContextCurrent(window);
    glViewport(0, 0, xs, ys);
}

static void error_callback(int error, const char *description)
{
    puts(description);
}

void initDisplay(GLWindow *g)
{
    int h = g->HEIGHT;
    int w = g->WIDTH;

    /// Initialize GLFW
    glfwInit();

    glfwSetErrorCallback(error_callback);

    // Open screen OpenGL window
    if (!(g->windows = glfwCreateWindow(g->WIDTH, g->HEIGHT, "Main", NULL, NULL)))
    {
        glfwTerminate();
        fprintf(stderr, "Window creation error...\n");
        abort();
    }

    glfwSetWindowAspectRatio(g->windows, 4, 3);

    glfwMakeContextCurrent(g->windows);
    setupGL(g, g->WIDTH, g->HEIGHT);

    glfwSwapInterval(1);            // We need vsync

    glfwGetWindowSize(g->windows, &w, &h);

    glfwSetWindowUserPointer(g->windows, g->keyArray);

    glfwSetKeyCallback(g->windows, kbHandler);
    glfwSetWindowSizeCallback(g->windows, sizeHandler);
}

static void drawPixel(GLWindow *gw, uint32_t x, uint32_t y, uint32_t colour)
{
    uint8_t r, g, b, a;

    uint32_t offset = (y * gw->WIDTH * 4U) + 4U * x;

    if ((x > (uint32_t)gw->WIDTH) || (y > (uint32_t)gw->HEIGHT))
    {
        return;
    }

    b = colour & 0xFF;
    g = (colour >> 8) & 0xFF;
    r = (colour >> 16) & 0xFF;
    a = (colour >> 24) & 0xFF;

    gw->videoMemory[offset + 0] = a;
    gw->videoMemory[offset + 1] = r;
    gw->videoMemory[offset + 2] = g;
    gw->videoMemory[offset + 3] = b;
}

static void drawLine(GLWindow *g, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t colour)
{
    printf("%s:%s(%p, %d, %d, %d, %d, %d) @ %d\n", __FILE__, __func__,
            g, x0, y0, x1, y1, colour,
            __LINE__);
    int32_t d, dx, dy, aincr, bincr, xincr, yincr;
    int32_t x, y;
    if (abs((int32_t)x1 - x0) < abs((int32_t)y1 - y0))
    {
        /* parcours par l'axe vertical */
        if (y0 > y1)
        {
            drawLine(g, x1, y1, x0, y0, colour);
            goto exit;
        }

        xincr = x1 > x0 ? 1 : -1;
        dy = y1 - y0;
        dx = abs((int32_t)x1 - x0);
        d = 2 * dx - dy;
        aincr = 2 * ((int32_t)dx - dy);
        bincr = 2 * dx;
        x = x0;
        y = y0;

        drawPixel(g, x, y, colour);

        for (y = y0 + 1 ; y <= y1 ; y++)
        {
            if (d >= 0)
            {
                x += xincr;
                d += aincr;
            }
            else
            {
                d += bincr;
            }

            drawPixel(g, x, y, colour);
        }

    }
    else
    {
        /* parcours par l'axe horizontal */
        if (x0 > x1)
        {
            drawLine(g, x1, y1, x0, y0, colour);
            goto exit;
        }
        yincr = y1 > y0 ? 1 : -1;
        dx = x1 - x0;
        dy = abs((int32_t)y1 - y0);
        d = 2 * dy - dx;
        aincr = 2 * (dy - dx);
        bincr = 2 * dy;
        x = x0;
        y = y0;

        drawPixel(g, x, y, colour);

        for (x = x0 + 1 ; x <= x1 ; ++x)
        {
            if (d >= 0)
            {
                y += yincr;
                d += aincr;
            }
            else
            {
                d += bincr;
            }

            drawPixel(g, x, y, colour);
        }
    }

exit:
    return;
}

static void drawCircle(GLWindow *g, int xc, int yc, int radius, uint32_t colour)
{
    int f = 1 - radius;
    int ddF_x = 0;
    int ddF_y = -2 * radius;
    int x = 0;
    int y = radius;
    int pX, pY;

    pX = xc;
    pY = yc + radius;
    drawPixel(g, pX, pY, colour);
    pY -= (2 * radius);
    drawPixel(g, pX, pY, colour);
    pY += radius;
    pX += radius;
    drawPixel(g, pX, pY, colour);
    pX -= (2 * radius);
    drawPixel(g, pX, pY, colour);

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x + 1;
        pX = xc + x;
        pY = yc + y;
        drawPixel(g, pX, pY, colour);
        pX = xc - x;
        pY = yc + y;
        drawPixel(g, pX, pY, colour);
        pX = xc + x;
        pY = yc - y;
        drawPixel(g, pX, pY, colour);
        pX = xc - x;
        pY = yc - y;
        drawPixel(g, pX, pY, colour);
        pX = xc + y;
        pY = yc + x;
        drawPixel(g, pX, pY, colour);
        pX = xc - y;
        pY = yc + x;
        drawPixel(g, pX, pY, colour);
        pX = xc + y;
        pY = yc - x;
        drawPixel(g, pX, pY, colour);
        pX = xc - y;
        pY = yc - x;
        drawPixel(g, pX, pY, colour);
    }
}

static void drawRect(GLWindow *g, uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint32_t colour)
{
    printf("%s:%s(%p, %d, %d, %d, %d, %d) @ %d\n", __FILE__, __func__,
           g, x0, y0, w, h, colour,
           __LINE__);
    drawLine(g, x0, y0, x0 + w, y0, colour);
    drawLine(g, x0 + w, y0, x0 + w, y0 + h, colour);
    drawLine(g, x0 + w, y0 + h, x0, y0 + h, colour);
    drawLine(g, x0, y0 + h, x0, y0, colour);
}

static void drawFillrect(GLWindow *g, uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint32_t colour)
{
    printf("%s:%s(%p, %d, %d, %d, %d, %d) @ %d\n", __FILE__, __func__,
           g, x0, y0, w, h, colour,
           __LINE__);
    uint32_t i;

    for (i = 0 ; i < h ; i++)
    {
        drawLine(g, x0, y0 + i, x0 + w, y0 + i, colour);
    }
}

void clearScreen(GLWindow *g)
{
    memset(g->videoMemory, 0, sizeof(uint8_t) * g->WIDTH * g->HEIGHT * 4);
}

void updateScreen(GLWindow *g)
{
    /* Update windows code */
    glfwMakeContextCurrent(g->windows);
    ShowScreen(g, g->WIDTH, g->HEIGHT);
    glfwSwapBuffers(g->windows);
    glfwPollEvents();
}

void updateScreenAndWait(GLWindow *g)
{
    while (glfwGetKey(g->windows, GLFW_KEY_ESCAPE) != GLFW_PRESS)
    {
        updateScreen(g);

        glfwPollEvents();
    }
    while (glfwGetKey(g->windows, GLFW_KEY_ESCAPE) != GLFW_RELEASE)
    {
        glfwPollEvents();
    }
}

GLWindow mainWindow;

int graphics_init()
{
    GLWindowInitEx(&mainWindow, 256, 240);
    initDisplay(&mainWindow);
    clearScreen(&mainWindow);
    updateScreen(&mainWindow);
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
    *w = mainWindow.WIDTH;
    *h = mainWindow.HEIGHT;
    return 0;
}

int graphics_drawRect(uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint32_t colour)
{
    printf("%s:%s(%d, %d, %d, %d, %d) @ %d\n", __FILE__, __func__,
           x0, y0, w, h, colour,
           __LINE__);
    drawRect(&mainWindow, x0, y0, w, h, colour);
    return 0;
}

int graphics_drawFillrect(int x0, int y0, int w, int h, uint32_t colour)
{
    printf("%s:%s(%d, %d, %d, %d, %d) @ %d\n", __FILE__, __func__,
           x0, y0, w, h, colour,
           __LINE__);
    drawFillrect(&mainWindow, x0, y0, w, h, colour);
    return 0;
}

int graphics_drawpixel(long x, long y, long color)
{
    drawPixel(&mainWindow, x, y, getColour(color));
    return 0;
}

int graphics_drawCircle(int xc, int yc, int radius, uint32_t colour)
{
    drawCircle(&mainWindow, xc, yc, radius, colour);
    return 0;
}

int graphics_drawline(uint32_t x, uint32_t y, uint32_t x1, uint32_t y1, uint32_t colour)
{
    printf("%s:%s(%d, %d, %d, %d, %d) @ %d\n", __FILE__, __func__,
           x, y, x1, y1, colour,
           __LINE__);
    drawLine(&mainWindow, x, y, x1, y1, getColour(colour));
    return 0;
}

int graphics_blit(long x, long y, long w, long h)
{
    /* Just pool for events, no graphic thing to be done ATM */
    glfwPollEvents();
    return 0;
}

int getKeyStatus(int key)
{
    return mainWindow.keyArray[key].curState;
}

/* Sync with 60Hz (or try to) */
void vsync(void)
{
    updateScreen(&mainWindow);
}