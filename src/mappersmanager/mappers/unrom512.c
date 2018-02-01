/*
 *  UNROM Mapper - The peTI-NESulator Project
 *  unrom.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 */

#include <ppu/ppu.h>
#include "unrom512.h"

static byte mirroring_set;
static byte loaded_vbank;
static byte loaded_pbank;

/*
 * not great but as we currently don't support higher than 8K VRAM, allocate it here as we can have
 * 32K on such a cart
 */
static uint8_t vram[32768];
void ppu_setPagePtr8k(byte page, byte *ptr);

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

    ppu_setPagePtr8k(0x00, vram + (loaded_vbank * 8 * 1024));
    set_prom_bank_16k(0x8000, loaded_pbank);
}

static void unrom512_MapperWriteHook(byte Addr, byte Value)
{
    mirroring_set = (Value >> 7) & 0x01;
    loaded_vbank  = (Value >> 5) & 0x03;
    loaded_pbank  = (Value     ) & 0x1F;

    printf(">> P:%d | V:%d | M:%d <<\n", loaded_pbank, loaded_vbank, mirroring_set);

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
