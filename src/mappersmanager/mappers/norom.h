/*
 *  NOROM Mapper - The peTI-NESulator Project
 *  norom.c
 *
 *  Created by Manoël Trapier.
 *  Copyright (c) 2002-2019 986-Studio.
 *
 */

#define __TINES_MAPPERS__

#include <mappers/manager.h>

int norom_InitMapper(NesCart *cart);
int norom_MapperIRQ(int cycledone);
void norom_MapperDump(FILE *fp);