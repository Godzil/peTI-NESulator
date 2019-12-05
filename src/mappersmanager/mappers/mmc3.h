/*
 *  MMC3 Mapper - The peTI-NESulator Project
 *  mmc3.h
 *
 *  Created by Manoël Trapier.
 *  Copyright (c) 2002-2019 986-Studio.
 *
 */

#define __TINES_MAPPERS__

#include <mappers/manager.h>

void mmc3_MapperDump(FILE *fp);
int mmc3_InitMapper(NesCart *cart);
int mmc3_MapperIRQ(int cycledone);