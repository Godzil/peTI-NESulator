/*
 *  PPU emulation - The TI-NESulator Project
 *  ppu.h
 *  
 *  Define and emulate the PPU (Picture Processing Unit) of the real NES
 * 
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2007 986Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-04-26 18:47:34 +0200 (jeu, 26 avr 2007) $
 *  $Author: mtrapier $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/ppu.h $
 *  $Revision: 46 $
 *
 */

#ifndef PPU_H
#define PPU_H

#include <types.h>

typedef struct PPU_Sprite_
{
    byte y;
    byte tileid;
    byte flags;
    byte x;
} PPU_Sprite;

/*
PPU must be initialized after memory initialisation..
*/
int ppu_init();

int ppu_hblank(int scanline);

byte ppu_readReg(byte id);

void ppu_writeReg(byte id, byte val);

void ppu_fillSprRamDMA(byte value);

#define PPU_MIRROR_HORIZTAL 0
#define PPU_MIRROR_VERTICAL 1

#define PPU_SCREEN_000 0
#define PPU_SCREEN_400 1
#define PPU_SCREEN_800 2
#define PPU_SCREEN_C00 3

#define PPU_SCMODE_SINGLE 0
#define PPU_SCMODE_NORMAL 1
#define PPU_SCMODE_FOURSC 2

void ppu_setMirroring(byte direction);
void ppu_setSingleScreen(byte screen);
void ppu_setScreenMode(byte mode);


PPU_Sprite ppu_getSprite(unsigned short i);

unsigned char ppu_memoryRead(byte page, byte addr);
void          ppu_memoryWrite(byte page, byte addr, byte value);

void ppu_debugSprites();
void ppu_debugColor();

#endif
