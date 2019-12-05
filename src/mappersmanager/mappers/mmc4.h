/*
 *  MMC4 Mapper - The peTI-NESulator Project
 *  mmc4.h
 *
 *  Created by Manoël Trapier.
 *  Copyright (c) 2002-2019 986-Studio.
 *
 */

#define __TINES_MAPPERS__

#include <mappers/manager.h>

void mmc4_MapperDump(FILE *fp);
int mmc4_InitMapper(NesCart *cart);