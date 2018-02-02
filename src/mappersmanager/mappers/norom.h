/*
 *  NOROM Mapper - The peTI-NESulator Project
 *  norom.c
 *
 *  Created by Manoël TRAPIER.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 */

#define __TINES_MAPPERS__

#include <mappers/manager.h>

int norom_InitMapper(NesCart *cart);
int norom_MapperIRQ(int cycledone);
void norom_MapperDump(FILE *fp);