/*
 *  mmc1.h
 *  TI-NESulator.X
 *
 *  Created by Manoël Trapier on 02/12/07.
 *  Copyright 2007 986 Corp. All rights reserved.
 *
 */

#define __TINES_MAPPERS__
#include <mappers/manager.h>

int mmc1_InitMapper  (NesCart *cart);
int mmc1_MapperIRQ   (int cycledone);
void mmc1_MapperDump ();
void mmc1_MapperWriteHook(register byte Addr, register byte Value); 