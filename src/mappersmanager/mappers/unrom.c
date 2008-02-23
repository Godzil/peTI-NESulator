/*
 *  UNROM Mapper - The TI-NESulator Project
 *  unrom.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2008 986Corp. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

#include "unrom.h"

unsigned char unrom_load_vbank;

void unrom_MapperWriteHook(byte Addr, byte Value);

int unrom_InitMapper(NesCart * cart) 
{
    int i;
    
    set_prom_bank_16k(0xC000, 0);
    set_prom_bank_16k(0x8000, GETLAST16KBANK(cart)); /* Set the last one */

    if (Cart->VROMSize > 0)
        set_vrom_bank_8k(0x0000,0);

    unrom_load_vbank = 0;
    
    /* Register the write hook */
    for (i = 0x80; i < 0x100; i++)
    {
        set_page_wr_hook(i, unrom_MapperWriteHook);
        set_page_writeable(i, true);
    }
    
    return 0;
} 


void unrom_MapperWriteHook(byte Addr, byte Value) 
{    
    set_vrom_bank_8k(0x0000,Value);
    unrom_load_vbank = Value;
} 

void unrom_MapperDump(FILE *fp)
{
    fprintf(fp,"unrom: vbank:%d\n",unrom_load_vbank);
}
