/*
 *  AOROM Mapper - The TI-NESulator Project
 *  aorom.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

#define __TINES_MAPPERS__
#include <mappers/manager.h>

int aorom_InitMapper(NesCart * cart);
void aorom_MapperDump(FILE *fp);