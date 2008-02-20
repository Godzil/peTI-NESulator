/*
 *  UNROM Mapper - The TI-NESulator Project
 *  unrom.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2007 986Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-04-16 01:55:35 +0200 (lun, 16 avr 2007) $
 *  $Author: godzil $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/unrom.h $
 *  $Revision: 39 $
 *
 */
 
unsigned char unrom_load_vbank;

void unrom_MapperWriteHook(register byte Addr, register byte Value);

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


void unrom_MapperWriteHook(register byte Addr, register byte Value) 
{    
    set_vrom_bank_8k(0x0000,Value);
    unrom_load_vbank = Value;
} 

void unrom_MapperDump(FILE *fp)
{
    fprintf(fp,"unrom: vbank:%d\n",unrom_load_vbank);
}
