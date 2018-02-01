/*
 *  Mappers manager & facilities - The peTI-NESulator Project
 *  mappers.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2008 986Corp. All rights reserved.
 *
 */

#ifndef MAPPERS_H
#define MAPPERS_H

#include <types.h>
#include <stdio.h>
#include <NESCarts.h>

typedef int (*MapperInit)      (NesCart * cart);
typedef int (*MapperWriteHook) (register uint16_t Addr,
                                register uint8_t Value);
typedef int (*MapperIRQ)       (int cycledone);
typedef void (*MapperDump)     (FILE *fp);

#ifdef __TINES_MAPPERS__

#include <ppu/ppu.h>
#include <memory/manager.h>
#include <os_dependent.h>

extern NesCart *Cart;

/* Available functions for mappers */
#define GETLAST08KBANK(c) ((c->PROMSize>>13)-1)
#define GETLAST16KBANK(c) ((c->PROMSize>>14)-1)
#define GETLAST32KBANK(c) ((c->PROMSize>>15)-1)

void set_vrom_bank_1k(uint16_t addr,int slot);
void set_vrom_bank_2k(uint16_t addr,int slot);
void set_vrom_bank_4k(uint16_t addr,int slot);
void set_vrom_bank_8k(uint16_t addr, int slot);

void set_prom_bank_8k(uint16_t addr,int slot);
void set_prom_bank_16k(uint16_t addr,int slot);
void set_prom_bank_32k(uint16_t addr,int slot);

#else /* __TINES_MAPPERS__ */

/* Available functions outside of mappers */

void mapper_list();
int  mapper_init(NesCart *cart);

extern MapperIRQ       mapper_irqloop;
extern MapperDump      mapper_dump;
extern MapperWriteHook mapper_hook;

#endif /* __TINES_MAPPERS__ */

void map_sram();     /* Map SRAM   */
void unmap_sram();   /* Unmap SRAM */

#endif
