/*
 *  AOROM Mapper - The peTI-NESulator Project
 *  aorom.c
 *
 *  Created by Manoël TRAPIER.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 */

#include "aorom.h"

uint8_t aorom_load_bank;

void aorom_MapperWriteHook(register uint8_t Addr, register uint8_t Value);

extern uint8_t *ppu_mem_nameTables;

int aorom_InitMapper(NesCart *cart)
{
    int i;

    set_prom_bank_32k(0x8000, 0);

    ppu_setScreenMode(PPU_SCMODE_SINGLE);

    aorom_load_bank = 0;

    /* Register the write hook */
    for (i = 0x80 ; i < 0x100 ; i++)
    {
        set_page_wr_hook(i, aorom_MapperWriteHook);
        set_page_writeable(i, true);
    }


    return 0;

}

void aorom_MapperWriteHook(register uint8_t Addr, register uint8_t Value)
{
    int BankNb;

    if (Value & (1 << 4))
    {
        ppu_setSingleScreen(PPU_SCREEN_000);
    }
    else
    {
        ppu_setSingleScreen(PPU_SCREEN_400);
    }

    BankNb = Value & 0x0F;

    aorom_load_bank = BankNb;

    //console_printf(Console_Default, "aorom: Asking bank %d - NT is 0x%04X\n",BankNb,(Value&0x10)?0x2400:0x2000);
    set_prom_bank_32k(0x8000, BankNb);
}

void aorom_MapperDump(FILE *fp)
{
    fprintf(fp, "aorom: bank:%d\n", aorom_load_bank);
}
