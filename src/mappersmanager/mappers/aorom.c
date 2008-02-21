/*
 *  AOROM Mapper - The TI-NESulator Project
 *  aorom.c
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2008 986Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-04-26 18:47:34 +0200 (jeu, 26 avr 2007) $
 *  $Author: mtrapier $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/aorom.h $
 *  $Revision: 46 $
 *
 */

#include "aorom.h"

unsigned char aorom_load_bank;

void aorom_MapperWriteHook(register byte Addr, register byte Value);

extern byte *ppu_mem_nameTables;

int aorom_InitMapper(NesCart * cart) 
{    
    int i;
    
    set_prom_bank_32k(0x8000,0);
  
    ppu_setScreenMode(PPU_SCMODE_SINGLE);
    
    aorom_load_bank = 0;

    /* Register the write hook */
    for (i = 0x80; i < 0x100; i++)
    {
        set_page_wr_hook(i, aorom_MapperWriteHook);
        set_page_writeable(i, true);
    }
    

    return 0;
    
} 

void aorom_MapperWriteHook(register byte Addr, register byte Value) 
{
    int BankNb;

        if (Value & (1 << 4))
            ppu_setSingleScreen(PPU_SCREEN_000);
        else
            ppu_setSingleScreen(PPU_SCREEN_400);

        BankNb = Value & 0x0F;

        aorom_load_bank = BankNb;

        //printf("aorom: Asking bank %d (giving %d & %d) - mirror is %d\n",BankNb,BankNb,(Value<<1)+1,Value&0x0F);
        set_prom_bank_32k(0x8000,BankNb);
} 

void aorom_MapperDump(FILE *fp)
{
    fprintf(fp,"aorom: bank:%d\n",aorom_load_bank);
}
