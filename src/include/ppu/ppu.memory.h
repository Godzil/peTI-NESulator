/*
 *  PPU Memory manager - The peTI-NESulator Project
 *  ppu.memory.h - Inspired from the memory manager of the Quick6502 Project.
 *
 *  Created by Manoel Trapier on 12/04/07.
 *  Copyright 2003-2008 986 Corp. All rights reserved.
 *
 */

#ifdef __TINES_PPU_INTERNAL__

int ppu_initMemory();

void ppu_setPagePtr  (uint8_t page, uint8_t *ptr);
void ppu_setPagePtr1k(uint8_t page, uint8_t *ptr);
void ppu_setPagePtr2k(uint8_t page, uint8_t *ptr);
void ppu_setPagePtr4k(uint8_t page, uint8_t *ptr);
void ppu_setPagePtr8k(uint8_t page, uint8_t *ptr);

void ppu_memoryDumpState(FILE *fp);

uint8_t ppu_readMemory(uint8_t page, uint8_t addr);
void ppu_writeMemory(uint8_t page, uint8_t addr, uint8_t value);

void ppu_setPageGhost(uint8_t page, uint8_t value, uint8_t ghost);

#else
#error Must only be included inside the PPU code
#endif
