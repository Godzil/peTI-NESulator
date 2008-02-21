/*
 *  PPU Memory manager - The TI-NESulator Project
 *  ppu.memory.h - Inspired from the memory manager of the Quick6502 Project.
 *
 *  Created by Manoël Trapier on 12/04/07.
 *  Copyright 2003-2008 986 Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-05-24 15:11:55 +0200 (jeu, 24 mai 2007) $
 *  $Author: mtrapier $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/ppu/ppu.memory.h $
 *  $Revision: 53 $
 *
 */

#ifdef __TINES_PPU_INTERNAL__

int ppu_initMemory();

void ppu_setPagePtr  (byte page, byte *ptr);
void ppu_setPagePtr1k(byte page, byte *ptr);
void ppu_setPagePtr2k(byte page, byte *ptr);
void ppu_setPagePtr4k(byte page, byte *ptr);
void ppu_setPagePtr8k(byte page, byte *ptr);

void ppu_memoryDumpState(FILE *fp);

byte ppu_readMemory(byte page, byte addr);
void ppu_writeMemory(byte page, byte addr, byte value);

#else
#error Must only be included inside the PPU code
#endif
