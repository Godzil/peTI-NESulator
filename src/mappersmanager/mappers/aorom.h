/*
 *  AOROM Mapper - The TI-NESulator Project
 *  aorom.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2007 986Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-04-26 18:47:34 +0200 (jeu, 26 avr 2007) $
 *  $Author: mtrapier $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/aorom.h $
 *  $Revision: 46 $
 *
 */

#define __TINES_MAPPERS__
#include <mappers/manager.h>

int aorom_InitMapper(NesCart * cart);
void aorom_MapperDump(FILE *fp);