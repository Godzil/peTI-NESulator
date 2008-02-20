/*
 *  Mappers manager & facilities - The TI-NESulator Project
 *  mappers.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2007 986Corp. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

#ifndef MAPPERS_H
#define MAPPERS_H

#include <types.h>
#include <stdio.h>
#include <NESCarts.h>

typedef int (*MapperInit)      (NesCart * cart);
typedef int (*MapperWriteHook) (register unsigned short Addr, 
                                register unsigned char Value);
typedef int (*MapperIRQ)       (int cycledone);
typedef void (*MapperDump)     ();

#ifdef __TINES_MAPPERS__

extern NesCart *Cart;

/* Available functions for mappers */
#define GETLAST08KBANK(c) ((c->PROMSize>>13)-1)
#define GETLAST16KBANK(c) ((c->PROMSize>>14)-1)
#define GETLAST32KBANK(c) ((c->PROMSize>>15)-1)

void map_sram();     /* Map SRAM   */
void unmap_sram();   /* Unmap SRAM */

void set_vrom_bank_1k(unsigned short addr,int slot);
void set_vrom_bank_2k(unsigned short addr,int slot);
void set_vrom_bank_4k(unsigned short addr,int slot);
void set_vrom_bank_8k(unsigned short addr, int slot);

void set_prom_bank_8k(unsigned short addr,int slot);
void set_prom_bank_16k(unsigned short addr,int slot);
void set_prom_bank_32k(unsigned short addr,int slot);

#else /* __TINES_MAPPERS__ */

/* Available functions outside of mappers */

       void mapper_list       ();
       int  mapper_init       (NesCart *cart);
extern int  (*mapper_irqloop) (int cyclodone);
extern void (*mapper_dump)    (FILE *fp);

#endif /* __TINES_MAPPERS__ */

#endif
