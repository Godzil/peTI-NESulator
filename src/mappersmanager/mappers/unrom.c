/*
 *  UNROM Mapper - The peTI-NESulator Project
 *  unrom.h
 *
 *  Created by Manoël Trapier.
 *  Copyright (c) 2002-2019 986-Studio.
 *
 */

#include "unrom.h"

uint8_t unrom_load_vbank;

void unrom_MapperWriteHook(uint8_t Addr, uint8_t Value);

int unrom_InitMapper(NesCart *cart)
{
    int i;

    set_prom_bank_16k(0xC000, 0);
    set_prom_bank_16k(0x8000, GETLAST16KBANK(cart)); /* Set the last one */

    if (Cart->VROMSize > 0)
    {
        set_vrom_bank_8k(0x0000, 0);
    }

    unrom_load_vbank = 0;

    /* Register the write hook */
    for (i = 0x80 ; i < 0x100 ; i++)
    {
        set_page_wr_hook(i, unrom_MapperWriteHook);
        set_page_writeable(i, true);
    }

    return 0;
}


void unrom_MapperWriteHook(uint8_t Addr, uint8_t Value)
{
    set_vrom_bank_8k(0x0000, Value);
    unrom_load_vbank = Value;
}

void unrom_MapperDump(FILE *fp)
{
    fprintf(fp, "unrom: vbank:%d\n", unrom_load_vbank);
}
