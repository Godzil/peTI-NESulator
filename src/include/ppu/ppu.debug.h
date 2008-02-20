/*
 *  PPU debug utilities - The TI-NESulator Project
 *  ppu.debug.h
 *
 *  Created by Manoël Trapier on 12/04/07.
 *  Copyright 2003-2007 986 Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-05-24 15:11:55 +0200 (jeu, 24 mai 2007) $
 *  $Author: mtrapier $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/ppu/ppu.debug.h $
 *  $Revision: 53 $
 *
 */

#ifdef __TINES_PPU_INTERNAL__

void ppu_dumpPalette(int x, int y);
void ppu_dumpPattern(int xd, int yd);
void ppu_dumpNameTable(int xd, int yd);
void ppu_dumpAttributeTable(int xd, int yd);

#else
#error Must only be included inside the PPU code
#endif
