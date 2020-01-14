/*
 *  FbLib graphic library
 *
 *  Created by Manoël TRAPIER.
 *  Copyright (c) 2003-2019 986-Studio. All rights reserved.
 *
 */

#ifndef _FBLIB_INCLUDE_TEXT_H
#define _FBLIB_INCLUDE_TEXT_H

typedef struct FBLibFont
{
   char      *name;       /* Font name. */
   int        height;     /* Height in pixels. */
   int        index_mask; /* ((1 << N) - 1). */
   int       *offset;     /* (1 << N) offsets into index. */
   int       *index;
   uint32_t  *content;
   //void      *private;
} FBLibFont;

/* ? */
FBLibFont *load_psf(char *filename);

void graphics_text_line(int x, int y, int w, int charw, uint32_t color, int valign, void *font, char *text);

int graphics_text_ex(int x, int y, int w, int h,
                     void *font,
                     uint32_t bgcolor, uint32_t fgcolor,
                     char valign, char halign,
                     uint16_t options,
                     void *format, ...);

void graphics_get_text_size(int *width, int *height,
                            const FBLibFont  *font,
                            const char *text);

void graphics_draw_text (int x, int y,
                         uint32_t colour,
                         const FBLibFont *font,
                         const char *text);

#define TEXT_VALIGN_LEFT   (1)
#define TEXT_VALIGN_RIGHT  (2)
#define TEXT_VALIGN_CENTER (3)

#define TEXT_HALIGN_TOP    (1)
#define TEXT_HALIGN_CENTER (2)
#define TEXT_HALIGN_BOTTOM (3)

#define TEXT_OPT_WORDWRAP  (1<<0)

#define TEXT_DEFAULTFONT   ((void*)(1))
#define TEXT_SMALLFONT     ((void*)(2))
#define TEXT_LARGEFONT     ((void*)(3))

#endif /* _FBLIB_INCLUDE_TEXT_H */