/*
 *  UNROM Mapper - The peTI-NESulator Project
 *  unrom.h
 *
 *  Created by Manoël Trapier.
 *  Copyright (c) 2002-2019 986-Studio.
 *
 */

#define __TINES_MAPPERS__

#include <mappers/manager.h>

int unrom_InitMapper(NesCart *cart);
void unrom_MapperDump(FILE *fp);