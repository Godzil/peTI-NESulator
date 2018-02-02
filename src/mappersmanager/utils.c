/*
 *  Mapper facilities - The peTI-NESulator Project
 *  mappers.c
 *
 *  Created by Manoël TRAPIER.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 */ 

//#define DEBUG_VROM_BANK_SWITCH
//#define DEBUG_PROM_BANK_SWITCH

#include <stdio.h>
#include <string.h>

#include <NESCarts.h>
#include <ppu/ppu.h>
#include <mappers/manager.h>
#include <memory/manager.h>

#define __TINES_PPU_INTERNAL__
#include <ppu/ppu.memory.h>
#undef __TINES_PPU_INTERNAL__

extern NesCart *Cart;

extern char MapperWantIRQ;

/*
 * Here are some function useful for mappers
 */

void set_vrom_bank_1k(uint16_t addr,int slot)
{
#ifdef DEBUG_VROM_BANK_SWITCH
    console_printf(Console_Default, "Change vrom 1k bank 0x%X to slot %d\n",addr,slot);
#endif
    ppu_setPagePtr1k((addr>>8)&0xFF, Cart->VROMBanks + (slot * 1024));
}

void set_vrom_bank_2k(uint16_t addr,int slot)
{
#ifdef DEBUG_VROM_BANK_SWITCH
    console_printf(Console_Default, "Change vrom 2k bank 0x%X to slot %d\n",addr,slot);
#endif
    ppu_setPagePtr2k((addr>>8)&0xFF, Cart->VROMBanks + (slot * 2 * 1024));
}

void set_vrom_bank_4k(uint16_t addr,int slot)
{
#ifdef DEBUG_VROM_BANK_SWITCH
    console_printf(Console_Default, "Change vrom 4k bank 0x%X to slot %d\n",addr,slot);
#endif
    ppu_setPagePtr4k((addr>>8)&0xFF, Cart->VROMBanks + (slot * 4 * 1024));
}

void set_vrom_bank_8k(uint16_t addr, int slot)
{
#ifdef DEBUG_VROM_BANK_SWITCH
    console_printf(Console_Default, "Change vrom 8k bank 0x%X to slot %d\n",addr,slot);
#endif
    ppu_setPagePtr8k(0x00, Cart->VROMBanks + (slot * 8 * 1024));
}

/*-----------*/

void set_prom_bank_8k(uint16_t addr,int slot)
{
#ifdef DEBUG_PROM_BANK_SWITCH
    console_printf(Console_Default, "Change prom 8k bank 0x%X to slot %d\n",addr,slot);
#endif
    set_page_ptr_8k(addr >> 8, Cart->PROMBanks + (slot * 8 * 1024));
}

void set_prom_bank_16k(uint16_t addr,int slot)
{
#ifdef DEBUG_PROM_BANK_SWITCH
    console_printf(Console_Default, "Change prom 16k bank @ 0x%X [0x%X] to slot 0x%X\n",addr, addr>>8,slot);
#endif
    set_page_ptr_16k(addr >> 8, Cart->PROMBanks + (slot * 16 * 1024));
}

void set_prom_bank_32k(uint16_t addr,int slot)
{ /* addr may not be different from 0x8000 !*/
  /* Anyway I don't use it */
#ifdef DEBUG_PROM_BANK_SWITCH
    console_printf(Console_Default, "Change prom 32k bank 0x%X to slot %d\n",addr,slot);
#endif
    set_page_ptr_32k(addr >> 8, Cart->PROMBanks + (slot * 32 * 1024));
}


void map_sram()
{
    int i;
    for (i = 0x60; i < 0x80; i++)
    {
        set_page_readable(i,true);
        set_page_writeable(i,true);
    }
}

void unmap_sram()
{
    int i;
    for (i = 0x60; i < 0x80; i++)
    {
        set_page_readable(i,false);
        set_page_writeable(i,false);
    }
}
