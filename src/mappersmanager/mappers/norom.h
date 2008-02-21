/*
 *  NOROM Mapper - The TI-NESulator Project
 *  norom.c
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2008 986Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-04-16 01:55:35 +0200 (lun, 16 avr 2007) $
 *  $Author: godzil $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/unrom.h $
 *  $Revision: 39 $
 *
 */

#define __TINES_MAPPERS__
#include <mappers/manager.h>

int norom_InitMapper  (NesCart *cart);
int norom_MapperIRQ   (int cycledone);
void norom_MapperDump ();