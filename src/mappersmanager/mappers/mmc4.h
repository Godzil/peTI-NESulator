/*
 *  MMC4 Mapper - The peTI-NESulator Project
 *  mmc4.h
 *
 *  Created by Manoël TRAPIER.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 */

#define __TINES_MAPPERS__

#include <mappers/manager.h>

void mmc4_MapperDump(FILE *fp);
int mmc4_InitMapper(NesCart *cart);