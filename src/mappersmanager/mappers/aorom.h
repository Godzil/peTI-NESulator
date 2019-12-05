/*
 *  AOROM Mapper - The peTI-NESulator Project
 *  aorom.h
 *
 *  Created by Manoël Trapier.
 *  Copyright (c) 2002-2019 986-Studio.
 *
 */

#define __TINES_MAPPERS__

#include <mappers/manager.h>

int aorom_InitMapper(NesCart *cart);
void aorom_MapperDump(FILE *fp);