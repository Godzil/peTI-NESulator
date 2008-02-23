/*
 *  PPU Memory manager - The TI-NESulator Project
 *  ppu.memory.h - Inspired from the memory manager of the Quick6502 Project.
 *
 *  Created by Manoel Trapier on 12/04/07.
 *  Copyright 2003-2008 986 Corp. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
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
