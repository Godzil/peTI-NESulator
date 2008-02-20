/*
 *  norom.h
 *  TI-NESulator.X
 *
 *  Created by Manoël Trapier on 25/10/07.
 *  Copyright 2007 986 Corp. All rights reserved.
 *
 */

#define __TINES_MAPPERS__
#include <mappers/manager.h>

int norom_InitMapper  (NesCart *cart);
int norom_MapperIRQ   (int cycledone);
void norom_MapperDump ();
void norom_MapperWriteHook(register byte Addr, register byte Value);