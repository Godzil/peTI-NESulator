/*
 *  CNROM Mapper - The peTI-NESulator Project
 *  cnrom.h
 *
 *  Created by Manoël TRAPIER.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 */

#define __TINES_MAPPERS__

#include <mappers/manager.h>

int cnrom_InitMapper(NesCart *cart);
void cnrom_MapperDump(FILE *fp);