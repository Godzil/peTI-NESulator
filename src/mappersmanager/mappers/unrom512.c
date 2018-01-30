/*
 *  UNROM Mapper - The TI-NESulator Project
 *  unrom.h
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

#include <ppu/ppu.h>
#include "unrom512.h"

static byte mirroring_set;
static byte loaded_vbank;
static byte loaded_pbank;
static void unrom512_applyValues()
{
    /*if (mirroring_set)
    {
        ppu_setMirroring(PPU);
    }
    else
    {
        ppu_setMirroring(PPU_MIRROR_VERTICAL);
    }*/

    //set_vrom_bank_8k(0x0000, loaded_vbank);
    set_prom_bank_16k(0x8000, loaded_pbank);
}

static void unrom512_MapperWriteHook(byte Addr, byte Value)
{
    mirroring_set = (Value >> 7) & 0x01;
    loaded_vbank  = (Value >> 5) & 0x03;
    loaded_pbank  = (Value     ) & 0x1F;

    unrom512_applyValues();
}

int unrom512_InitMapper(NesCart * cart)
{
    int i;

    loaded_vbank = 0;
    loaded_pbank = 0;
    set_prom_bank_16k(0xC000, GETLAST16KBANK(cart));

    unrom512_applyValues();
    
    /* Register the write hook */
    for (i = 0x80; i < 0x100; i++)
    {
        set_page_wr_hook(i, unrom512_MapperWriteHook);
        set_page_writeable(i, true);
    }
    
    return 0;
} 

void unrom512_MapperDump(FILE *fp)
{
    fprintf(fp,"unrom512: vbank:%d pbank:%d\n", loaded_vbank, loaded_pbank);
}
