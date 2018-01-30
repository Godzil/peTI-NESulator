/*
 *  PPU emulation - The TI-NESulator Project
 *  ppu.h
 *  
 *  Define and emulate the PPU (Picture Processing Unit) of the real NES
 * 
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

#ifndef PPU_H
#define PPU_H

#include <types.h>

typedef struct PPU_Sprite_
{
    uint8_t y;
    uint8_t tileid;
    uint8_t flags;
    uint8_t x;
} PPU_Sprite;

/*
PPU must be initialized after memory initialisation..
*/
int ppu_init();

int ppu_hblank(uint16_t scanline);

uint8_t ppu_readReg(uint8_t id);

void ppu_writeReg(uint8_t id, uint8_t val);

void ppu_fillSprRamDMA(uint8_t value);

#define PPU_MIRROR_HORIZTAL 0
#define PPU_MIRROR_VERTICAL 1

#define PPU_SCREEN_000 0
#define PPU_SCREEN_400 1
#define PPU_SCREEN_800 2
#define PPU_SCREEN_C00 3

#define PPU_SCMODE_SINGLE 0
#define PPU_SCMODE_NORMAL 1
#define PPU_SCMODE_FOURSC 2

void ppu_setMirroring(uint8_t direction);
void ppu_setSingleScreen(uint8_t screen);
void ppu_setScreenMode(uint8_t mode);


PPU_Sprite ppu_getSprite(uint16_t i);

unsigned char ppu_memoryRead(uint8_t page, uint8_t addr);
void          ppu_memoryWrite(uint8_t page, uint8_t addr, uint8_t value);

void ppu_debugSprites();
void ppu_debugColor();

#endif
