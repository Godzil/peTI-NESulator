/*
 *  CNROM Mapper - The TI-NESulator Project
 *  cnrom.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2008 986Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-04-16 01:55:35 +0200 (lun, 16 avr 2007) $
 *  $Author: godzil $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/cnrom.h $
 *  $Revision: 39 $
 *
 */

#define __TINES_MAPPERS__
#include <mappers/manager.h>

int cnrom_InitMapper(NesCart * cart);
void cnrom_MapperDump(FILE *fp);