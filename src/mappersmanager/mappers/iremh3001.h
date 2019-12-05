/*
 *  IREMH3001 Mapper - The peTI-NESulator Project
 *  iremh3001.h
 *
 *  Created by Manoël Trapier.
 *  Copyright (c) 2002-2019 986-Studio.
 *
 */

#define __TINES_MAPPERS__

#include <mappers/manager.h>

int iremh3001_InitMapper(NesCart *cart);
void iremh3001_MapperDump(FILE *fp);
int iremh3001_MapperIRQ(int cycledone);