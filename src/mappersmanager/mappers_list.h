/*
 *  Mapper list - The TI-NESulator Project
 *  mappers_list.h
 *
 *  Created by Manoel TRAPIER on 25/10/07.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

/* This file could be generated from the mappers directory... */
#include "mappers/norom.h"
#include "mappers/aorom.h"
#include "mappers/unrom.h"
#include "mappers/cnrom.h"

#include "mappers/unrom512.h"

#include "mappers/iremh3001.h"

#include "mappers/mmc1.h"
#include "mappers/mmc3.h"
#include "mappers/mmc4.h"

Mapper Mappers[] = {
{ 0 , "No Mapper", norom_InitMapper, norom_MapperIRQ, norom_MapperDump },
{ 7 , "AOROM", aorom_InitMapper, norom_MapperIRQ, aorom_MapperDump },   
{ 2 , "CNROM", cnrom_InitMapper, norom_MapperIRQ, cnrom_MapperDump },   
{ 3 , "UNROM", unrom_InitMapper, norom_MapperIRQ, unrom_MapperDump },

{ 1 , "MMC1", mmc1_InitMapper, norom_MapperIRQ, mmc1_MapperDump },   
{ 4 , "MMC3", mmc3_InitMapper, mmc3_MapperIRQ, mmc3_MapperDump },   
{ 10, "MMC4", mmc4_InitMapper, norom_MapperIRQ, mmc4_MapperDump },

{ 30, "UNROM512", unrom512_InitMapper, norom_MapperIRQ, unrom512_MapperDump },
   
{ 65, "Irem H3001", iremh3001_InitMapper, iremh3001_MapperIRQ, iremh3001_MapperDump },   


{ 100, "Floppy Disk System", NULL, norom_MapperIRQ, norom_MapperDump },   
   /* EOL tag */
{ 0, NULL, NULL, NULL, NULL }
};
