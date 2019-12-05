/*
 *  CNROM Mapper - The peTI-NESulator Project
 *  cnrom.h
 *
 *  Created by Manoël Trapier.
 *  Copyright (c) 2002-2019 986-Studio.
 *
 */

#define __TINES_MAPPERS__

#include <mappers/manager.h>

int cnrom_InitMapper(NesCart *cart);
void cnrom_MapperDump(FILE *fp);