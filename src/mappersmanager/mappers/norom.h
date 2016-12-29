/*
 *  NOROM Mapper - The TI-NESulator Project
 *  norom.c
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

int norom_InitMapper  (NesCart *cart);
int norom_MapperIRQ   (int cycledone);
void norom_MapperDump ();