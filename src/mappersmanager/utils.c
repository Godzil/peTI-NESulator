/*
 *  Mapper facilities - The TI-NESulator Project
 *  mappers.c
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2008 986Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-05-31 18:00:41 +0200 (jeu, 31 mai 2007) $
 *  $Author: mtrapier $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/mappers.c $
 *  $Revision: 56 $
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
 Here are some fonction useful for mappers
 */

void set_vrom_bank_1k(unsigned short addr,int slot)
{
#ifdef DEBUG_VROM_BANK_SWITCH
    printf("Change vrom 1k bank 0x%X to slot %d\n",addr,slot);
#endif
    ppu_setPagePtr1k((addr>>8)&0xFF, Cart->VROMBanks + (slot * 1024));
//    memcpy(ppu.Memory+addr, Cart->VROMBanks + (slot * 1024), 0x0400);
}

void set_vrom_bank_2k(unsigned short addr,int slot)
{
#ifdef DEBUG_VROM_BANK_SWITCH
    printf("Change vrom 2k bank 0x%X to slot %d\n",addr,slot);
#endif
    ppu_setPagePtr2k((addr>>8)&0xFF, Cart->VROMBanks + (slot * 2 * 1024));
//    memcpy(ppu.Memory+addr, Cart->VROMBanks + (slot * 2 * 1024), 0x0800);
}

void set_vrom_bank_4k(unsigned short addr,int slot)
{
#ifdef DEBUG_VROM_BANK_SWITCH
    printf("Change vrom 4k bank 0x%X to slot %d\n",addr,slot);
#endif
    ppu_setPagePtr4k((addr>>8)&0xFF, Cart->VROMBanks + (slot * 4 * 1024));
//    memcpy(ppu.Memory+addr, Cart->VROMBanks + (slot * 4 * 1024), 0x1000);
}

void set_vrom_bank_8k(unsigned short addr, int slot)
{
#ifdef DEBUG_VROM_BANK_SWITCH
    printf("Change vrom 8k bank 0x%X to slot %d\n",addr,slot);
#endif
    ppu_setPagePtr8k(0x00, Cart->VROMBanks + (slot * 8 * 1024));
//    memcpy(ppu.Memory, Cart->VROMBanks + (slot * 8 * 1024) , 0x2000);    
}

/*-----------*/

void set_prom_bank_8k(unsigned short addr,int slot)
{
#ifdef DEBUG_PROM_BANK_SWITCH
    printf("Change prom 8k bank 0x%X to slot %d\n",addr,slot);
#endif
    set_page_ptr_8k(addr >> 8, Cart->PROMBanks + (slot * 8 * 1024));
}

void set_prom_bank_16k(unsigned short addr,int slot)
{
#ifdef DEBUG_PROM_BANK_SWITCH
    printf("Change prom 16k bank @ 0x%X [0x%X] to slot 0x%X\n",addr, addr>>8,slot);
#endif
    set_page_ptr_16k(addr >> 8, Cart->PROMBanks + (slot * 16 * 1024));
}

void set_prom_bank_32k(unsigned short addr,int slot)
{ /* addr may not be different from 0x8000 !*/
  /* Anyway I don't use it */
#ifdef DEBUG_PROM_BANK_SWITCH
    printf("Change prom 32k bank 0x%X to slot %d\n",addr,slot);
#endif
    set_page_ptr_32k(addr >> 8, Cart->PROMBanks + (slot * 32 * 1024));
/*    set_page_ptr_16k(0x80, Cart->PROMBanks[(slot<<1)]);
    set_page_ptr_16k(0xC0, Cart->PROMBanks[(slot<<1)+1]);*/
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
