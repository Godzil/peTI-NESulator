/*
 *  FbLib graphic library
 *
 *  Created by Manoël TRAPIER.
 *  Copyright (c) 2003-2019 986-Studio. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include <os_dependent.h>

#define DEFAULT_FONT "default_font.psf"
FBLibFont *defaultFont = NULL;

/* Function will fail, if no string terminator */
static int getNextWordLen(char *str)
{
    int ret = 0, i;
    /* Word delimiters */
    char word_lim[] = { ' ', '\t', '\n', 0 };

    while (1)
    {
        for (i = 0 ; word_lim[i] != 0 ; i++)
        {
            if (*str == word_lim[i])
            {
                return ret;
            }
        }
        str++;
        ret++;
    }
}

void graphics_text_line(int x, int y, int w, int charw, uint32_t color, int valign, void *font, char *text)
{
    uint32_t len = strlen(text);

    switch (valign)
    {
    default:
    case TEXT_VALIGN_LEFT:
        graphics_draw_text(x, y, color, (const FBLibFont *)font, text);
        break;
    case TEXT_VALIGN_CENTER:
        graphics_draw_text(x + ((w - len * charw) / 2), y, color, (const FBLibFont *)font, text);
        break;
    case TEXT_VALIGN_RIGHT:
        graphics_draw_text(x + (w - (len * charw)), y, color, (const FBLibFont *)font, text);
        break;
    }
}

/* Currently halign is not honored, but valign is */
int graphics_text_ex(int x, int y, int w, int h,
                     void *font,
                     uint32_t bgcolor, uint32_t fgcolor,
                     char valign, char halign,
                     uint16_t options,
                     void *format, ...)
{
    char string[1024];
    char line[300];
    int charWidth, charHeight;
    int textPos = 0;
    int wordLen = 0;
    int nextWordLen = 0;

    va_list va;
    uint16_t curColPos = 0, curLinePos = 0;
    uint16_t maxCharPerLine, maxTextLine;

    FBLibFont *userFont = font;
    if (defaultFont == NULL)
    {
        defaultFont = load_psf(DEFAULT_FONT);
    }

    if (font == NULL)
    {
        userFont = defaultFont;
    }

    /* Do some usefull calculation */
    /* We use fixed size font */
    graphics_get_text_size(&charWidth, &charHeight, userFont, "A");
    maxCharPerLine = w / charWidth;
    maxTextLine = h / charHeight;

    /* Now convert to a useable string */
    va_start(va, format);
    vsnprintf(string, 1024, format, va);
    va_end(va);

    /* Fill rect with bg color */
    graphics_drawFillrect(x, y, w, h, bgcolor);

    /* Now fill as much as possible */
    memset(line, 0, 300);
    while (curLinePos < maxTextLine)
    {
        if (options & TEXT_OPT_WORDWRAP)
        {
            /* Do thoses check only one time per word, not per characters */
            if (wordLen <= 0)
            {
                /* check if next word is too large for width */
                nextWordLen = getNextWordLen(&string[textPos]);
                //printf("\nNextword len = %d", nextWordLen);
                if (nextWordLen <= maxCharPerLine)
                {
                    if ((curColPos + nextWordLen) > maxCharPerLine)
                    {
                        /* Go next line... */
                        line[curColPos] = 0;
                        graphics_text_line(x, y + curLinePos * charHeight, w, charWidth, fgcolor, valign, userFont,
                                           line);
                        curColPos = 0;
                        curLinePos++;
                        memset(line, 0, 300);
                    }
                }
                wordLen = nextWordLen;
            }
            /* Now when the word is too long for a line, it will be automatically wrapped to the next line */
        }

        if ((string[textPos] == '\n') || (string[textPos] == '\r'))
        {
            textPos++;
            line[curColPos] = 0;
            graphics_text_line(x, y + curLinePos * charHeight, w, charWidth, fgcolor, valign, userFont, line);
            curColPos = 0;
            curLinePos++;
            memset(line, 0, 300);
        }
        else if (string[textPos] == 0)
        {
            line[curColPos] = 0;
            graphics_text_line(x, y + curLinePos * charHeight, w, charWidth, fgcolor, valign, userFont, line);
            goto exit;
        }
        else if (curColPos >= maxCharPerLine)
        {
            /* display the line */
            line[curColPos] = 0;
            graphics_text_line(x, y + curLinePos * charHeight, w, charWidth, fgcolor, valign, userFont, line);
            /* skip until a "\n" (and exit is "\0" found)) */
            if (options & TEXT_OPT_WORDWRAP)
            {
                curColPos = 0;
                curLinePos++;
                memset(line, 0, 300);
            }
            else
            {
                while (1)
                {
                    if ((string[textPos] == '\r') || (string[textPos] == '\n'))
                    {
                        curColPos = 0;
                        curLinePos++;
                        memset(line, 0, 300);
                        break;
                    }
                    else if (string[textPos] == 0)
                    {
                        goto exit;
                    }

                    textPos++;
                }
            }
        }
        else
        {
            line[curColPos++] = string[textPos++];
        }

        if (options & TEXT_OPT_WORDWRAP)
        {
            wordLen--;
        }
    }

exit:
    return 0;
}

void *fblib_loadfont(char *filename)
{
    return (void *)load_psf(filename);
}


/* PSF management */
#define PSF1_MAGIC0     0x36
#define PSF1_MAGIC1     0x04

#define PSF1_MODE512    0x01
#define PSF1_MODEHASTAB 0x02
#define PSF1_MODEHASSEQ 0x04
#define PSF1_MAXMODE    0x05

#define PSF1_SEPARATOR  0xFFFF
#define PSF1_STARTSEQ   0xFFFE

struct psf1_header
{
    unsigned char magic[2];     /* Magic number */
    unsigned char mode;         /* PSF font mode */
    unsigned char charsize;     /* Character size */
};

#define PSF2_MAGIC0     0x72
#define PSF2_MAGIC1     0xb5
#define PSF2_MAGIC2     0x4a
#define PSF2_MAGIC3     0x86

/* bits used in flags */
#define PSF2_HAS_UNICODE_TABLE 0x01

/* max version recognized so far */
#define PSF2_MAXVERSION 0

/* UTF8 separators */
#define PSF2_SEPARATOR  0xFF
#define PSF2_STARTSEQ   0xFE

struct psf2_header
{
    unsigned char magic[4];
    unsigned int version;
    unsigned int headersize;    /* offset of bitmaps in file */
    unsigned int flags;
    unsigned int length;        /* number of glyphs */
    unsigned int charsize;      /* number of bytes for each character */
    unsigned int height, width; /* max dimensions of glyphs */
    /* charsize = height * ((width + 7) / 8) */
};

static FBLibFont *load_psf1(char *filename, FILE *fp)
{
    struct psf1_header head;
    struct FBLibFont *font;
    fread(&head, sizeof(head), 1, fp);

    if ((head.magic[0] != PSF1_MAGIC0) || (head.magic[1] != PSF1_MAGIC1))
    {
        return NULL;
    }

    font = (FBLibFont *)malloc(sizeof(FBLibFont));

    if (font != NULL)
    {
        font->height = head.charsize;
        font->index_mask = 0xFF;


    }

    return NULL;
}

void printbin(uint32_t val, uint8_t bitlen)
{
    int i;
    for (i = 0 ; i < bitlen ; i++)
    {
        if (val & (1 << (bitlen - 1)))
        {
            printf("*");
        }
        else
        {
            printf("_");
        }
        val <<= 1;
    }
}

static FBLibFont *load_psf2(char *filename, FILE *fp)
{
    struct psf2_header head;
    struct FBLibFont *font, *ret = NULL;
    uint32_t charWidth;
    uint32_t i, j, k;
    uint8_t *bitmap;

    fread(&head, sizeof(head), 1, fp);

    if ((head.magic[0] != PSF2_MAGIC0) || (head.magic[1] != PSF2_MAGIC1) ||
        (head.magic[2] != PSF2_MAGIC2) || (head.magic[3] != PSF2_MAGIC3)
            )
    {
        goto exit;
    }

    font = (FBLibFont *)malloc(sizeof(FBLibFont));

    assert(head.width <= 32); /* For now, do not support font with width larger than 32 pixels */

    if (font != NULL)
    {
        font->height = head.height;

        bitmap = (uint8_t *)malloc(sizeof(uint8_t) * head.charsize * head.length);
        font->index_mask = 0xFF;
        font->offset = (int *)malloc(sizeof(int) * head.length);
        font->index = (int *)malloc(sizeof(int) * head.length * 3);
        font->content = (uint32_t *)malloc(sizeof(uint32_t) * head.length * head.height);

        charWidth = ((head.width + 7) / 8);

        assert(bitmap != NULL);
        assert(font->offset != NULL);
        assert(font->index != NULL);
        assert(font->content != NULL);

        fread(bitmap, sizeof(uint8_t), head.charsize * head.length, fp);

        for (i = 0 ; i < head.length ; i++)
        {
            font->offset[i] = i * 3;
            font->index[(i * 3) + 0] = head.width;
            font->index[(i * 3) + 1] = i * head.height;
            font->index[(i * 3) + 2] = 0;

            for (j = 0 ; j < head.height ; j++)
            {
                font->content[(i * head.height) + j] = 0;
                for (k = 0 ; k < charWidth ; k++)
                {
                    font->content[(i * head.height) + j] |=
                            (bitmap[(i * head.charsize) + (j * charWidth) + k]) << 8 * (3 - k);
                }
            }
        }
        ret = font;
        free(bitmap);
    }

exit:
    fclose(fp);
    return ret;
}

FBLibFont *load_psf(char *filename)
{
    FILE *fp;
    uint8_t byte;
    console_printf(Console_Default, "Loading font '%s'\n", filename);
    fp = fopen(filename, "rb");
    if (fp != NULL)
    {
        byte = fgetc(fp);
        rewind(fp);
        switch (byte)
        {
        default:
            fclose(fp);
            return NULL; // Unsuported format
        case PSF1_MAGIC0:
            return load_psf1(filename, fp);
        case PSF2_MAGIC0:
            return load_psf2(filename, fp);
        }
    }

    return NULL;
}

/* Font rendering code based on BOGL by Ben Pfaff */

static int fblib_draw_glyph(const FBLibFont *font, uint8_t wc, uint32_t **bitmap)
{
    int mask = font->index_mask;
    int i;

    for (;;)
    {
        for (i = font->offset[wc & mask] ; font->index[i] ; i += 3)
        {
            if ((font->index[i] & ~mask) == (wc & ~mask))
            {
                if (bitmap != NULL)
                {
                    *bitmap = &font->content[font->index[i + 1]];
                }
                return font->index[i] & mask;
            }
        }
    }
    return 0;
}

void graphics_get_text_size(int *width, int *height,
                            const FBLibFont *font,
                            const char *text)
{
    uint8_t *c = (uint8_t *)text;
    uint8_t wc;
    int k, n, w, h, mw;

    if (defaultFont == NULL)
    {
        defaultFont = load_psf(DEFAULT_FONT);
    }

    if (font == NULL)
    {
        font = defaultFont;
    }

    n = strlen(text);
    mw = h = w = 0;

    for (k = 0 ; k < n ; k++)
    {
        wc = *(c++);
        if (wc == '\n')
        {
            if (w > mw)
            {
                mw = 0;
            }

            h += font->height;
            continue;
        }

        w += fblib_draw_glyph(font, wc, NULL);
    }

    if (width != NULL)
    {
        *width = (w > mw) ? w : mw;
    }
    if (height != NULL)
    {
        *height = (h == 0) ? font->height : h;
    }
}

void graphics_draw_text(int x, int y,
                        uint32_t colour,
                        const FBLibFont *font,
                        const char *text)
{
    int32_t h, w, k, n, cx, cy, dx, dy;
    uint8_t *c = (uint8_t *)text;
    uint8_t wc;

    if (defaultFont == NULL)
    {
        defaultFont = load_psf(DEFAULT_FONT);
    }

    if (font == NULL)
    {
        font = defaultFont;
    }

    n = strlen(text);
    h = font->height;
    dx = dy = 0;

    for (k = 0 ; k < n ; k++)
    {
        uint32_t *glyph = NULL;
        wc = *(c++);

        if (wc == '\n')
        {
            dy += h;
            dx = 0;
            continue;
        }

        w = fblib_draw_glyph(font, wc, &glyph);

        if (glyph == NULL)
        {
            continue;
        }

        for (cy = 0 ; cy < h ; cy++)
        {
            uint32_t g = *glyph++;

            for (cx = 0 ; cx < w ; cx++)
            {
                if (g & 0x80000000)
                {
                    graphics_drawpixel(x + dx + cx, y + dy + cy, colour);
                }
                g <<= 1;
            }
        }

        dx += w;
    }
}

/* End of PSF */
