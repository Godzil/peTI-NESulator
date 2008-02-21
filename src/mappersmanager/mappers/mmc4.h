/*
 *  MMC4 Mapper - The TI-NESulator Project
 *  mmc4.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2007 986Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-05-31 18:00:41 +0200 (jeu, 31 mai 2007) $
 *  $Author: mtrapier $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/mmc4.h $
 *  $Revision: 56 $
 *
 */

#define __TINES_MAPPERS__
#include <mappers/manager.h>

void mmc4_MapperDump(FILE *fp);
int mmc4_InitMapper(NesCart * cart);
