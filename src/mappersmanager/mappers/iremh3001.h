/*
 *  IREMH3001 Mapper - The TI-NESulator Project
 *  iremh3001.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2007 986Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-04-16 01:55:35 +0200 (lun, 16 avr 2007) $
 *  $Author: godzil $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/iremh3001.h $
 *  $Revision: 39 $
 *
 */

#define __TINES_MAPPERS__
#include <mappers/manager.h>

int iremh3001_InitMapper(NesCart * cart);
void iremh3001_MapperDump(FILE *fp);
int iremh3001_MapperIRQ(int cycledone);