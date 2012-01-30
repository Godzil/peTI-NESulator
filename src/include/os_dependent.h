/*
 *  OS Dependent functions - The TI-NESulator Project
 *  os_dependent.h
 *
 *  Created by Manoel TRAPIER on 08/05/08.
 *  Copyright (c) 2003-2008 986Corp. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

#ifndef OS_DEPENDENT_H
#define OS_DEPENDENT_H

/* File related functions */
/* Graphics related functions */
int graphics_init();
int graphics_drawpixel(long x, long y, long color);
int graphics_blit(long x, long y, long w, long h);

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