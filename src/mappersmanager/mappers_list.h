/*
 *  mappers_list.h
 *  TI-NESulator.X
 *
 *  Created by Manoël Trapier on 25/10/07.
 *  Copyright 2007 986 Corp. All rights reserved.
 *
 */

/* This file could be generated from the mappers directory... */
#include "mappers/norom.h"
#include "mappers/mmc1.h"

Mapper Mappers[] = {
{ 0, "No Mapper", norom_InitMapper, norom_MapperIRQ, norom_MapperDump },
{ 1, "MMC1", mmc1_InitMapper, norom_MapperIRQ, mmc1_MapperDump },   

   /* EOL tag */
{ 0, NULL, NULL, NULL, NULL }
};
