/*
 *  CNROM Mapper - The TI-NESulator Project
 *  cnrom.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2007 986Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-04-16 01:55:35 +0200 (lun, 16 avr 2007) $
 *  $Author: godzil $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/cnrom.h $
 *  $Revision: 39 $
 *
 */

unsigned char cnrom_load_bank;

void cnrom_MapperWriteHook(register byte Addr, register byte Value);

int cnrom_InitMapper(NesCart * cart) 
{
    int i;
    
    set_prom_bank_16k(0x8000,  0);
    set_prom_bank_16k(0xC000, GETLAST16KBANK(cart)); /* Set the last one */
    cnrom_load_bank = 0;
    
    /* Register the write hook */
    for (i = 0x80; i < 0x100; i++)
    {
        set_page_wr_hook(i, cnrom_MapperWriteHook);
        set_page_writeable(i, true);
    }
    
    return 0;
} 


void cnrom_MapperWriteHook(register byte Addr, register byte Value) 
{
    set_prom_bank_16k(0x8000,Value);
    cnrom_load_bank = Value;
}
 
void cnrom_MapperDump(FILE *fp)
{
    fprintf(fp,"cnrom: bank:%d\n",cnrom_load_bank);
}
