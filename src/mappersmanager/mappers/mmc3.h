/*
 *  MMC3 Mapper - The TI-NESulator Project
 *  mmc3.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2008 986Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-05-02 18:37:41 +0200 (mer, 02 mai 2007) $
 *  $Author: mtrapier $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/mmc3.h $
 *  $Revision: 50 $
 *
 */

#define __TINES_MAPPERS__
#include <mappers/manager.h>

void mmc3_MapperDump(FILE *fp);
int mmc3_InitMapper(NesCart * cart); 
int mmc3_MapperIRQ(int cycledone);