/*
 *  TI-68k Loading external file functions - The peTI-NESulator Project
 *  ti68k/loadfile.c
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2008 986Corp. All rights reserved.
 *
 *  $LastChangedDate:$
 *  $Author:$
 *  $HeadURL:$
 *  $Revision:$
 *
 */

#define TIGCC_COMPAT
#include <tigcclib.h>

/* Map a file in memory */
void *LoadFilePtr(char * filename)
{
   void *RetPtr = NULL;
   FILE *fp;
   
   if ((fp = fopen(filename,"rb")) == NULL)
      return -1;
   
   /* TI Related stuff, very uggly, and need to be changed.. */
   HeapLock(fp->handle);
   RetPtr = 2 + HeapDeref(fp->handle);
   
   fclose (fp);
   
   return RetPtr;
}
