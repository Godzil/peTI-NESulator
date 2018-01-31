/*
 *  OS Dependent functions - The peTI-NESulator Project
 *  os_dependent.h
 *
 *  Created by Manoel TRAPIER on 08/05/08.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

#ifndef OS_DEPENDENT_H
#define OS_DEPENDENT_H

#include <stdint.h>

/* File related functions */
/* Graphics related functions */
int graphics_init();
int graphics_drawpixel(long x, long y, long color);
int graphics_blit(long x, long y, long w, long h);
int graphics_drawline(long x, long y, long x1, long y1, long color);

typedef struct Palette_t
{
   uint8_t r,g,b,a;
} Palette;

int getKeyStatus(int key);

/* Sound related functions */

/* IO functions */
void *LoadFilePtr(char * filename);

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

#endif /* OS_DEPENDENT_H */