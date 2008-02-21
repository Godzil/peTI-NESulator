/*
 *  MMC1 Mapper - The TI-NESulator Project
 *  mmc1.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2007 986Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-05-02 18:37:41 +0200 (mer, 02 mai 2007) $
 *  $Author: mtrapier $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/mmc1.h $
 *  $Revision: 50 $
 *
 */

#define __TINES_MAPPERS__
#include <mappers/manager.h>

int mmc1_InitMapper  (NesCart *cart);
int mmc1_MapperIRQ   (int cycledone);
void mmc1_MapperDump ();