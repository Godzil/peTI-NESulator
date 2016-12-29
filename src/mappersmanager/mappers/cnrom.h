/*
 *  CNROM Mapper - The TI-NESulator Project
 *  cnrom.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2016 986-Studio. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

#define __TINES_MAPPERS__
#include <mappers/manager.h>

int cnrom_InitMapper(NesCart * cart);
void cnrom_MapperDump(FILE *fp);