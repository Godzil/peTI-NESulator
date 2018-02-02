/*
 *  MMC3 Mapper - The peTI-NESulator Project
 *  mmc3.h
 *
 *  Created by Manoël TRAPIER.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 */

#define __TINES_MAPPERS__

#include <mappers/manager.h>

void mmc3_MapperDump(FILE *fp);
int mmc3_InitMapper(NesCart *cart);
int mmc3_MapperIRQ(int cycledone);