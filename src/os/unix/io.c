/*
 *  IO Manager - The peTI-NESulator Project
 *  os/macos/graphics.c
 *
 *  Created by Manoël Trapier on 04/01/09.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

#include <stdio.h>
#include <stdarg.h>

#include <os_dependent.h>

char LevelChar[] = { 'E', 'W', 'A', 'N', 'V',  'D'};

ConsoleLevel console_ActualLevel = Console_Default;

/* Actually nothing to do */
int console_init(ConsoleLevel DefaultLevel)
{
   console_ActualLevel = DefaultLevel;
   return 0;
}

/* Actually a simple printf with levels */
int console_vprintf(const ConsoleLevel level, const char *format, va_list ap)
{
   if (console_ActualLevel >= level)
      vprintf(format, ap);
   
   return 0;
}


int console_printf(const ConsoleLevel level, const char *format, ...)
{
   va_list ap;
   va_start(ap, format);

   console_vprintf(level, format, ap);

   va_end(ap);
   return 0;
}

int console_printf_d(const char *format, ...)
{
   va_list ap;
   va_start(ap, format);
   
   console_vprintf (Console_Debug, format, ap);

   va_end(ap);

   return 0;
}
