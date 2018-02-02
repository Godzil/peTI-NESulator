/*
 *  UNROM Mapper - The peTI-NESulator Project
 *  unrom.h
 *
 *  Created by Manoël TRAPIER.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 */

#define __TINES_MAPPERS__

#include <mappers/manager.h>

int unrom512_InitMapper(NesCart *cart);
void unrom512_MapperDump(FILE *fp);