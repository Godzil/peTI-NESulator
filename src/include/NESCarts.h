/*
 *  Cart manager - The TI-NESulator Project
 *  NESCart.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2008 986Corp. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 */

#ifndef NESCARTS_H
#define NESCARTS_H

#include <types.h>

#define iNES_MIRROR  0x01
#define iNES_BATTERY 0x02
#define iNES_TRAINER 0x04
#define iNES_4SCREEN 0x08

typedef struct NesCart_
{  
    unsigned long PROMSize, /* Size of PROM */
                  VROMSize; /* Size of VROM */
    char MapperID;          /* Mapper Type */
    byte Flags;
    char *FileName;
    byte *File;             /* Pointer on the file in memory */
    byte *PROMBanks;        /* Pointer on the first PROM */
    byte *VROMBanks;        /* Pointer on the first VROM */
} NesCart;

void DumpCartProperties();
int LoadCart(const char *filename, NesCart * cart);

#endif
