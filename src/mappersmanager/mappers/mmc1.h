/*
 *  MMC1 Mapper - The peTI-NESulator Project
 *  mmc1.h
 *
 *  Created by Manoël TRAPIER.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 */

#define __TINES_MAPPERS__

#include <mappers/manager.h>

int mmc1_InitMapper(NesCart *cart);
int mmc1_MapperIRQ(int cycledone);
void mmc1_MapperDump(FILE *fp);