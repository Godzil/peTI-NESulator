/*
 *  CNROM Mapper - The peTI-NESulator Project
 *  cnrom.c
 *
 *  Created by Manoël Trapier.
 *  Copyright (c) 2002-2019 986-Studio.
 *
 */

#include "cnrom.h"

uint8_t cnrom_load_bank;

void cnrom_MapperWriteHook(register uint8_t Addr, register uint8_t Value);

int cnrom_InitMapper(NesCart *cart)
{
    int i;

    set_prom_bank_16k(0x8000, 0);
    set_prom_bank_16k(0xC000, GETLAST16KBANK(cart)); /* Set the last one */
    cnrom_load_bank = 0;

    /* Register the write hook */
    for (i = 0x80 ; i < 0x100 ; i++)
    {
        set_page_wr_hook(i, cnrom_MapperWriteHook);
        set_page_writeable(i, true);
    }

    return 0;
}


void cnrom_MapperWriteHook(register uint8_t Addr, register uint8_t Value)
{
    set_prom_bank_16k(0x8000, Value);
    cnrom_load_bank = Value;
}

void cnrom_MapperDump(FILE *fp)
{
    fprintf(fp, "cnrom: bank:%d\n", cnrom_load_bank);
}
