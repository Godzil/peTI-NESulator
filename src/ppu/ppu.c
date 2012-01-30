/*
 *  PPU emulation - The TI-NESulator Project
 *  ppu.c
 *
 *  Define and emulate the PPU (Picture Processing Unit) of the real NES
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2008 986Corp. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

/* Allegro includes */
#ifdef __APPLE__
#define USE_CONSOLE
#include <Allegro/allegro.h>
#else
#define USE_CONSOLE
#include <allegro.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#define __TINES_PPU_INTERNAL__
#include <ppu/ppu.h>
#include <ppu/ppu.memory.h>
#include <ppu/ppu.debug.h>

#include <memory/manager.h>

#include <os_dependent.h>

#define __TINES_PLUGINS__
#include <plugins/manager.h>

extern int VBLANK_TIME;

extern BITMAP *Buffer;

volatile extern int frame;
volatile extern unsigned long IPS, FPS;

extern unsigned long ColorPalette[ 9 * 63 ];
extern short IRQScanHit;

BITMAP *VideoBuffer; /* The ppu will only write pixel to this, and then bliting 
                        this on the screen "surface" */

/* PPU sprite sorted by scanline */

/* Work as follow: 

3322 2222 2222 1111 1111 1100 0000 0000
1098 7654 3210 9876 5432 1098 7654 3210
---------------------------------------
AAAA AAAA TTTT TTTT xxxx XXXX YYYY YYYY
---------------------------------------
8421 8421 8421 8421 8421 8421 8421 8421

A = Sprite Attributes
x = reserved
T = Tile ID
X = X relative position
Y = Y absolute position

x = for future use

 */
unsigned long PPU_SpriteByScanLine[241][9];        /* There is 240 scanline and 8 sprite per scanline */
unsigned long PPU_NbSpriteByScanLine[241];         /* There is 240 scanline and 8 sprite per scanline */
unsigned long PPU_NbSpriteByScanLineOverFlow[241]; /* There is 240 scanline and 8 sprite per scanline */

#define PPU_SCANLINESPRITE_GET_ATTRS(sprt)  (((sprt)&0xFF000000) >> 24)
#define PPU_SCANLINESPRITE_GET_TILIDX(sprt) (((sprt)&0x00FF0000) >> 16)
#define PPU_SCANLINESPRITE_GET_RELY(sprt)   (((sprt)&0x00000F00) >>  8)
#define PPU_SCANLINESPRITE_GET_X(sprt)       ((sprt)&0x000000FF)

#define PPU_SCANLINESPRITE_SET_ATTRS(sprt, v)  sprt = (((sprt)&0x00FFFFFF) | (( (v) & 0xFF) << 24))
#define PPU_SCANLINESPRITE_SET_TILIDX(sprt, v) sprt = (((sprt)&0xFF00FFFF) | (( (v) & 0xFF) << 16))
#define PPU_SCANLINESPRITE_SET_RELY(sprt, v)   sprt = (((sprt)&0xFFFFF0FF) | (( (v) & 0x0F) <<  8))
#define PPU_SCANLINESPRITE_SET_X(sprt, v)      sprt = (((sprt)&0xFFFFFF00) | (  (v) & 0xFF       ))

/* PPU registers */

/* NT: Name Table */
byte PPU_Reg_NT;

/* AT: Attribute/Color Table */
byte PPU_Reg_AT;

/* FV: Fine Vertical Scroll latch/counter */
byte PPU_Reg_FV;

/* HV: Fine Horizontal Scroll latch/counter */
byte PPU_Reg_FH;

/* VT: Vertical Tile indev latch/counter */
byte PPU_Reg_VT;

/* HT: Horizontal Tile indev latch/counter */
byte PPU_Reg_HT;

/* V: Vertical Name Table Selection latch/counter */
byte PPU_Reg_V;

/* H: Horizontal Name Table Selection latch/counter */
byte PPU_Reg_H;

/* S: Playfield pattern table selection latch */
unsigned short PPU_Reg_S;

/* PAR: Picture Address Register */
byte PPU_Reg_PAR;

/* AR: Tile Attribute (palette select) value latch */
byte PPU_Reg_AR;

unsigned short PPU_Reg_Counter;


/* PPU Memory Areas */
byte *ppu_mem_nameTables;
byte *ppu_mem_patternTables;
byte *ppu_mem_paletteValues;

byte ppu_mem_spritesTable[0x100];
byte ppu_mem_sptrTablePtr;


/* Some other PPU "registers" */
byte ppu_VramAccessFlipFlop;

byte ppu_inVBlankTime;
byte ppu_spriteZeroHit;
byte ppu_scanlineSpriteOverflow;

byte ppu_bgColor;

/* CR #1 variables */
unsigned short ppu_spritePatternTable;
byte ppu_spriteSize;
byte ppu_addrIncrement;
byte ppu_execNMIonVBlank;

/* CR #2 variables */
byte ppu_spriteVisibility;
byte ppu_backgroundVisibility;
byte ppu_spriteClipping;
byte ppu_backgroundClipping;
byte ppu_displayType;

byte ppu_mirrorMode;
byte ppu_singleScreenMode;
byte ppu_screenMode;

#define PPU_MEM_PATTERNTABLES_SIZE 0x2000
#define PPU_MEM_NAMETABLE_SIZE 0x1000
#define PPU_MEM_PALETTEVALUES_SIZE 0x100 /* in fact its 20 but we must allocate a least one page */

#define PPU_SPRITE_FLAGS_VFLIP       ( 1 << 7 )
#define PPU_SPRITE_FLAGS_HFLIP       ( 1 << 6 )
#define PPU_SPRITE_FLAGS_BGPRIO      ( 1 << 5 )
#define PPU_SPRITE_FLAGS_UPPERCOLOR  ( 0x03 )

#define PPU_FLAG_SR_VBLANK     ( 1 << 7 )
#define PPU_FLAG_SR_SPRT0      ( 1 << 6 )
#define PPU_FLAG_SR_8SPRT      ( 1 << 5 ) 
#define PPU_FLAG_SR_RDWRALLOW  ( 1 << 4 ) 

#define PPU_CR1_SPRTSIZE       ( 1 << 5 )
#define PPU_CR1_EXECNMI        ( 1 << 7 )

#define PPU_CR2_BGVISIBILITY   ( 1 << 3 )
#define PPU_CR2_SPRTVISIBILITY ( 1 << 4 )

int ppu_init()
{
    int i;

    /*byte defaultColors[] = { 0x09,0x01,0x00,0x01,0x00,0x02,0x02,0x0D,0x08,0x10,0x08,0x24,0x00,0x00,0x04,0x2C,
                             0x09,0x01,0x34,0x03,0x00,0x04,0x00,0x14,0x08,0x3A,0x00,0x02,0x00,0x20,0x2C,0x08 };*/

    if (ppu_initMemory())
        return -1;

    /* Set ppu memory parameters */

    /* First: Allocate each memory zone */
    ppu_mem_patternTables = (byte*) malloc(PPU_MEM_PATTERNTABLES_SIZE);
    if (!ppu_mem_patternTables)
        return -1;

    ppu_mem_nameTables = (byte*) malloc(PPU_MEM_NAMETABLE_SIZE);
    if (!ppu_mem_nameTables)
        return -1;

    ppu_mem_paletteValues = (byte*) malloc(PPU_MEM_PALETTEVALUES_SIZE);
    if (!ppu_mem_paletteValues)
        return -1;

    console_printf(Console_Default, "ppu_mem_nameTables   :%p\n"
           "ppu_mem_patternTables:%p\n"
           "ppu_mem_paletteValues:%p\n",
           ppu_mem_nameTables,
           ppu_mem_patternTables,
           ppu_mem_paletteValues);

    /* Second: make the ppu memory manager point on the memory zones */
    ppu_setPagePtr8k(0x00, ppu_mem_patternTables);
    ppu_setPagePtr4k(0x20, ppu_mem_nameTables);
    ppu_setPagePtr  (0x3F, ppu_mem_paletteValues);

    for ( i = 0x00; i < 0x0F; i++ )
        ppu_setPageGhost(0x30 + i, true, 0x20 + i);

    /* Third: set registers to defaults */

    /* Now test the memory ! */

    /* Fille PPU memory with garbage */
    for (i = 0x0000; i < 0x2000 ; i++)
        ppu_mem_patternTables[i]    = rand()%0xFF;
    for (i = 0x0000; i < 0x1000 ; i++)
        ppu_mem_nameTables[i] = rand()%0xFF;
    for (i = 0x0000; i < 0x001F ; i++)
        ppu_mem_paletteValues[i] = rand()%0xFF;

    //memcpy(ppu_mem_paletteValues, defaultColors, 32);

    /* Set some other variables */
    ppu_VramAccessFlipFlop = 0;

    ppu_addrIncrement      = 1;
    ppu_spritePatternTable = 0;
    ppu_spriteSize         = 8;
    ppu_execNMIonVBlank    = 0;

    ppu_spriteVisibility     = 0;
    ppu_backgroundVisibility = 0;
    ppu_spriteClipping       = 0;
    ppu_backgroundClipping   = 0;
    ppu_displayType          = 0;

    ppu_inVBlankTime = 0;
    ppu_bgColor = 0;

    /* Set PPU registers on CPU side */
    set_page_rd_hook(0x20, ppu_readReg);
    set_page_wr_hook(0x20, ppu_writeReg);

    set_page_readable(0x20, true);
    set_page_writeable(0x20, true);


    /* Set PPU Ghost Registers */
    for(i = 0x21; i < 0x40; i++)
        set_page_ghost(i, true, 0x20);

    /* allocate the PPU Video memory */
    VideoBuffer = create_bitmap(256, 240);

    if (VideoBuffer == NULL)
        return -1;

    return 0;
}

void ppu_updateSpriteScanlineTable()
{
    int i, line, j, k;
    volatile int sprite_x, sprite_y, sprite_idx, sprite_attr;

    int curline;

    for (line = 0; line < 241; line ++)
    {
        PPU_NbSpriteByScanLine[line] = 0;
        PPU_NbSpriteByScanLineOverFlow[line] = 0;

        for (i = 0; i < 9; i++)
            PPU_SpriteByScanLine[line][i] = 0xFFFFFFFF;
    }

    for ( i = 0; i < 64; i ++)
    {
        /* Fill sprite_zzz variables */
        sprite_y    = ppu_mem_spritesTable[(i*4) + 0] + 1;
        sprite_idx  = ppu_mem_spritesTable[(i*4) + 1];
        sprite_attr = ppu_mem_spritesTable[(i*4) + 2] | ((i==0)?0x04:0); /* Add a flag for the sprite #0 */
        sprite_x    = ppu_mem_spritesTable[(i*4) + 3];

        /* For each line covered by the sprite */
        for (line = 0; line < ppu_spriteSize; line ++)
        {
            curline = line + sprite_y;

            if ((curline < 0) || (curline > 240))
                continue; /* Don't go beyond, this sprite go beyond the borders */

            if (PPU_NbSpriteByScanLine[curline] < 8)
                PPU_NbSpriteByScanLine[curline] ++;
            else
            {
                PPU_NbSpriteByScanLineOverFlow[curline] = 1;
                continue; /* We have 8 sprite in this line, don't continue */
            }
            if (((sprite_x+8) < 0) && ((sprite_x-8) > 256))
                continue; /* this sprite isn't either displayable */
            /* Now test if this sprite can be put in the sprite list */            
            for (j = 0; j <= PPU_NbSpriteByScanLine[curline]; j++)
            {
                /* sprite are ordered by their y value, so, the first time that
                   we have lower y value is where we need to put the sprite */
                if (sprite_x < PPU_SCANLINESPRITE_GET_X(PPU_SpriteByScanLine[curline][j]))
                {
                    /* move the j eme item and next to the right in the list, trashing
                       if needed the rightest item. */
                    for (k = 7; k >= j; k--)
                        PPU_SpriteByScanLine[curline][k] = PPU_SpriteByScanLine[curline][k-1];

                    PPU_SpriteByScanLine[curline][j] = 0;

                    PPU_SCANLINESPRITE_SET_ATTRS (PPU_SpriteByScanLine[curline][j], sprite_attr);
  
                    PPU_SCANLINESPRITE_SET_TILIDX(PPU_SpriteByScanLine[curline][j], sprite_idx);
  
                    PPU_SCANLINESPRITE_SET_RELY  (PPU_SpriteByScanLine[curline][j], curline - sprite_y);
  
                    PPU_SCANLINESPRITE_SET_X     (PPU_SpriteByScanLine[curline][j], sprite_x);
                    
                    break; /* Stop the for, we don't need to go further in the line list */
                }
            }
        }
    }
}

void ppu_setMirroring(byte direction)
{
    if (ppu_screenMode != PPU_SCMODE_NORMAL)
        return;

    if (ppu_mirrorMode == direction)
        return; /* Same value, no need to change! */

    switch(direction)
    {
    default:
        direction = PPU_MIRROR_HORIZTAL;
        ppu_mirrorMode = direction;

    case PPU_MIRROR_HORIZTAL: /* Horizontal */
        ppu_setPagePtr1k(0x20, ppu_mem_nameTables + 0x000);
        ppu_setPagePtr1k(0x24, ppu_mem_nameTables + 0x000);
        ppu_setPagePtr1k(0x28, ppu_mem_nameTables + 0x400);
        ppu_setPagePtr1k(0x2C, ppu_mem_nameTables + 0x400);
        break;
    case PPU_MIRROR_VERTICAL: /* Vertical */
        ppu_setPagePtr1k(0x20, ppu_mem_nameTables + 0x000);
        ppu_setPagePtr1k(0x24, ppu_mem_nameTables + 0x400);
        ppu_setPagePtr1k(0x28, ppu_mem_nameTables + 0x000);
        ppu_setPagePtr1k(0x2C, ppu_mem_nameTables + 0x400);
        break;
    }
    ppu_mirrorMode = direction;
}

void ppu_setSingleScreen(byte screen)
{
    if (ppu_screenMode != PPU_SCMODE_SINGLE)
        return;
    if (ppu_singleScreenMode == screen)
        return; /* Same value, no need to change! */

    switch(screen)
    {
    default:
        screen = PPU_SCREEN_000;
        ppu_singleScreenMode = screen;

    case PPU_SCREEN_000: /* 0x2000 */
        ppu_setPagePtr1k(0x20, ppu_mem_nameTables + 0x000);
        ppu_setPagePtr1k(0x24, ppu_mem_nameTables + 0x000);
        ppu_setPagePtr1k(0x28, ppu_mem_nameTables + 0x000);
        ppu_setPagePtr1k(0x2C, ppu_mem_nameTables + 0x000);
        break;

    case PPU_SCREEN_400: /* 0x2400 */
        ppu_setPagePtr1k(0x20, ppu_mem_nameTables + 0x400);
        ppu_setPagePtr1k(0x24, ppu_mem_nameTables + 0x400);
        ppu_setPagePtr1k(0x28, ppu_mem_nameTables + 0x400);
        ppu_setPagePtr1k(0x2C, ppu_mem_nameTables + 0x400);
        break;

    case PPU_SCREEN_800: /* 0x2800 */
        ppu_setPagePtr1k(0x20, ppu_mem_nameTables + 0x800);
        ppu_setPagePtr1k(0x24, ppu_mem_nameTables + 0x800);
        ppu_setPagePtr1k(0x28, ppu_mem_nameTables + 0x800);
        ppu_setPagePtr1k(0x2C, ppu_mem_nameTables + 0x800);
        break;

    case PPU_SCREEN_C00: /* 0x2C00 */
        ppu_setPagePtr1k(0x20, ppu_mem_nameTables + 0xC00);
        ppu_setPagePtr1k(0x24, ppu_mem_nameTables + 0xC00);
        ppu_setPagePtr1k(0x28, ppu_mem_nameTables + 0xC00);
        ppu_setPagePtr1k(0x2C, ppu_mem_nameTables + 0xC00);
        break;
    }
    ppu_singleScreenMode = screen;
}

/* Let set display to
   Single screen (1 NT with mirroring)
   Normal screen (2 NT with mirroring)
   Four   screen (4 NT without mirroring) */
void ppu_setScreenMode(byte mode)
{
    if (ppu_screenMode == mode)
        return; /* Same value, no need to change! */

    ppu_screenMode = mode;

    switch(mode)
    {
    case PPU_SCMODE_SINGLE: /* Single screen (1 NT with mirroring) */
        ppu_setSingleScreen(~ppu_singleScreenMode);
        break;

    default:
        mode = PPU_SCMODE_NORMAL;
        ppu_screenMode = mode;

    case PPU_SCMODE_NORMAL: /* Normal screen (2 NT with mirroring) */
        ppu_setMirroring(~ppu_mirrorMode);
        break;

    case PPU_SCMODE_FOURSC: /* Four   screen (4 NT withou mirroring) */
        ppu_setPagePtr1k(0x20, ppu_mem_nameTables + 0x000);
        ppu_setPagePtr1k(0x24, ppu_mem_nameTables + 0x400);
        ppu_setPagePtr1k(0x28, ppu_mem_nameTables + 0x800);
        ppu_setPagePtr1k(0x2C, ppu_mem_nameTables + 0xC00);
        break;
    }
}

/* update whole counters */
void ppu_updateCounters()
{
/*
+---------------+-----------------------------------------------+
|               |+===++=++=++=====++=====++===++=++========++==+|
|PPU registers  || FV||V||H||   VT||   HT|| FH||S||     PAR||AR||
|PPU counters   |+---++-++-++-----++-----++===++=++========++==+|
|               |+===++=++=++=====++=====+                      |
+---------------+-----------------------------------------------+
|2007 access    |  DC  B  A  98765  43210                       |
+===============+===============================================+

8421 8421 8421 8421
-------------------
1111 1100 0000 0000
5432 1098 7654 3210
_AAA BCDD DDDE EEEE

*/
    PPU_Reg_Counter  = (PPU_Reg_FV & 0x07) << 12;
    PPU_Reg_Counter |= PPU_Reg_V           << 11;
    PPU_Reg_Counter |= PPU_Reg_H           << 10;
    PPU_Reg_Counter |= PPU_Reg_VT          << 5;
    PPU_Reg_Counter |= PPU_Reg_HT;
}

int ppu_hblank(int scanline)
{
    int i, j;
    byte pixelColor = 0x42;
    byte BgColor = 0x42;
    byte SpriteColor = 0x42;
    unsigned short addr;
    byte value;
    unsigned short tmp_HHT = 0;
    unsigned short tmp_VVTFV = 0;
    unsigned long CurrentSprite;
    byte SpriteVFlip;
      
    if (scanline == 0)
    {
      ppu_bgColor =  ppu_readMemory(0x3F,00);

      if ((ppu_spriteVisibility != 0) || (ppu_backgroundVisibility != 0))
          ppu_updateCounters();
    }

    if (scanline < 240)
    {

        /* For each PPU pixel of this scanline */
        for (i = 0; i < 256; i ++)
        {
            /* Set the current pixel color to the bg color */
            pixelColor = ppu_readMemory(0x3F,00);

            /* Compute current pixel bg color if bg is visible */
            if (ppu_backgroundVisibility == 1)
            {
                addr  = (PPU_Reg_Counter & 0x0C00);
                addr  = addr | 0x03C0;
                addr |= (PPU_Reg_Counter >> 4 ) & 0x0038;
                addr |= (PPU_Reg_Counter >> 2 ) & 0x0007;

                PPU_Reg_AR = ppu_readMemory(0x20 | ((addr>>8) & 0x0F), addr& 0xFF);

                PPU_Reg_AR = PPU_Reg_AR >> (((PPU_Reg_Counter >> 4 ) & 0x04)|((PPU_Reg_Counter ) & 0x02));
                PPU_Reg_AR = (PPU_Reg_AR<<2) & 0x0C;

                PPU_Reg_PAR = ppu_readMemory(0x20 | ((PPU_Reg_Counter>>8) & 0x0F), PPU_Reg_Counter& 0xFF);

                addr  =  PPU_Reg_S;
                addr |= ((PPU_Reg_PAR & 0xFF) << 4);
                addr |= ((PPU_Reg_Counter >> 12)  & 0x07);
                
                value = ppu_readMemory((addr >> 8) , addr        );
                BgColor = (value & (1 << (7-(i + PPU_Reg_FH) % 8)))?0x01:0;

                value = ppu_readMemory((addr >> 8) , addr | 0x08 );
                BgColor |= (value & (1 << (7-(i + PPU_Reg_FH) % 8)))?0x02:0;

                if (BgColor > 0x00)
                {
                    BgColor |= PPU_Reg_AR;
                    BgColor &= 0x0F;

                    pixelColor = ppu_readMemory(0x3F, BgColor);
                }

                if (((i + PPU_Reg_FH)%8) == 7)
                {
                    tmp_HHT = ((PPU_Reg_Counter >> 5) & 0x0020) |
                               (PPU_Reg_Counter & 0x001F);
                    tmp_HHT = (tmp_HHT + 1) & 0x003F;

                    /* Reassemble with HT & H */
                    PPU_Reg_Counter = (PPU_Reg_Counter & 0xFBE0) |
                                      ((tmp_HHT & 0x0020) << 5)  |
                                      (tmp_HHT & 0x001F);
                }
            }

            /* Now calculate if there is a sprite here and sprite visibility is on */
            if ((ppu_spriteVisibility == 1) && 
                (PPU_NbSpriteByScanLine[scanline] != 0))
            {
                /* scan each sprite on this line to find the one (or more) that is on this pixel */
                for (j = 0; j < PPU_NbSpriteByScanLine[scanline]; j++)
                {
                    /* they are orderer by X, so if this one is too far on the right
                       it's not need to go further */
                    CurrentSprite = PPU_SpriteByScanLine[scanline][j];

                    if (PPU_SCANLINESPRITE_GET_X(CurrentSprite) > i)
                       break; /* break the current for */

                    if ((PPU_SCANLINESPRITE_GET_X(CurrentSprite) + 8) < i)
                       continue; /* Not this one too (too far on the left) try next one*/

                    /* Ok if we arrive here, the current sprite is on the good position */
                    /* Does the sprite is a BG or FG sprite ? */
                    
                        /* Ok we could now get the sprite current pixel color */
                        /* Read sprite scanline pattern */                        
                        SpriteVFlip = PPU_SCANLINESPRITE_GET_ATTRS(CurrentSprite) & PPU_SPRITE_FLAGS_VFLIP;
                        
                        if (ppu_spriteSize == 8)
                        {
                           addr = (PPU_SCANLINESPRITE_GET_TILIDX(CurrentSprite) << 4) + ppu_spritePatternTable;
                        }
                        else
                        {
                           if (PPU_SCANLINESPRITE_GET_RELY(CurrentSprite) < 8)
                              addr = (((PPU_SCANLINESPRITE_GET_TILIDX(CurrentSprite)&0xFE) + (SpriteVFlip?1:0)) << 4) + ((PPU_SCANLINESPRITE_GET_TILIDX(CurrentSprite)&0x01)?0x1000:0x0000);
                           else
                              addr = (((PPU_SCANLINESPRITE_GET_TILIDX(CurrentSprite)&0xFE) + (SpriteVFlip?0:1)) << 4) + ((PPU_SCANLINESPRITE_GET_TILIDX(CurrentSprite)&0x01)?0x1000:0x0000);
                        }
                        if (SpriteVFlip)
                        {  
                           addr += 7;
                           addr -= (PPU_SCANLINESPRITE_GET_RELY(CurrentSprite) % 8);
                        }
                        else
                           addr += (PPU_SCANLINESPRITE_GET_RELY(CurrentSprite) % 8);


                        if (PPU_SCANLINESPRITE_GET_ATTRS(CurrentSprite) & PPU_SPRITE_FLAGS_HFLIP)
                        {
                           value = ppu_readMemory((addr >> 8) , addr        );
                           SpriteColor  = (value & (1 << (i-PPU_SCANLINESPRITE_GET_X(CurrentSprite))))?0x01:0;

                           value = ppu_readMemory((addr >> 8) , addr | 0x08 );
                           SpriteColor |= (value & (1 << (i-PPU_SCANLINESPRITE_GET_X(CurrentSprite))))?0x02:0;
                        }
                        else
                        {
                           value = ppu_readMemory((addr >> 8) , addr        );
                           SpriteColor  = (value & (1 << (7-(i-PPU_SCANLINESPRITE_GET_X(CurrentSprite)))))?0x01:0;

                           value = ppu_readMemory((addr >> 8) , addr | 0x08 );
                           SpriteColor |= (value & (1 << (7-(i-PPU_SCANLINESPRITE_GET_X(CurrentSprite)))))?0x02:0;
                        }

                        if (SpriteColor > 0x00)
                        {
                            SpriteColor |= ((PPU_SCANLINESPRITE_GET_ATTRS(CurrentSprite) & PPU_SPRITE_FLAGS_UPPERCOLOR) << 2);
                            SpriteColor &= 0x0F;
                        }

                        if ((PPU_SCANLINESPRITE_GET_ATTRS(CurrentSprite) & 0x04) &&
                            (SpriteColor != 0x00) && (BgColor != 0x00))
                        {
                           ppu_spriteZeroHit = 1;
                        }
                     if ( ( (PPU_SCANLINESPRITE_GET_ATTRS(CurrentSprite) & PPU_SPRITE_FLAGS_BGPRIO) && (BgColor == 0x0000)) ||
                         (!(PPU_SCANLINESPRITE_GET_ATTRS(CurrentSprite) & PPU_SPRITE_FLAGS_BGPRIO)) )
                    {
                        if (SpriteColor != 0x00) pixelColor = ppu_readMemory(0x3F, (0x10 + SpriteColor));
                    }
               }
            }
           
        /* Set to monochrome if needed */
        if (ppu_displayType)
            pixelColor &= 0x30;
           
        /* draw the pixel */
        _putpixel(VideoBuffer, i, scanline, pixelColor);
       }
       
       if (ppu_backgroundVisibility || ppu_spriteVisibility)
          if (PPU_NbSpriteByScanLineOverFlow[scanline] == 1)
               ppu_scanlineSpriteOverflow = 1;
            
       //blit(VideoBuffer, screen, 0, scanline, 0, scanline, 256, 1);


        if (ppu_backgroundVisibility == 1)
        {

        tmp_VVTFV = ((PPU_Reg_Counter >> 3 ) & 0x0100) | /* V */
                    ((PPU_Reg_Counter >> 2 ) & 0x00F8) | /* VT */
                    ((PPU_Reg_Counter >> 12) & 0x0007);  /* FV */

        tmp_VVTFV++;
        if ((tmp_VVTFV&0x0F8) == 0xF0)
        {
            tmp_VVTFV &= ~0x0F8;
            tmp_VVTFV ^= 0x100;
        }

        PPU_Reg_Counter = ( PPU_Reg_Counter & 0x041F)   |
                          ((tmp_VVTFV & 0x0100 ) << 3 ) | /* V */
                          ((tmp_VVTFV & 0x00F8 ) << 2 ) | /* VT */
                          ((tmp_VVTFV & 0x0007 ) << 12);  /* FV */

        /* Update H & HT */
        PPU_Reg_Counter = (PPU_Reg_Counter & ~0x041F) |
                          (PPU_Reg_H << 10)           |
                           PPU_Reg_HT;
      }
    }
        /* Increment only V & VT & FV*/
/*

8421 8421 8421 8421
-------------------
1111 1100 0000 0000
5432 1098 7654 3210
_AAA BCDD DDDE EEEE

 xxx x xx xxx        : vvtfv = 7BE0
      x      x xxxx  : hht

        B DDDD DAAA  : vvtfv
            CE EEEE  : hht

A = FV
B = V
C = H
D = VT
E = HT


*/
    if (scanline == 239)
    {
         ppu_inVBlankTime = 1;

       textprintf(Buffer, font, 260, 3, 4, "FPS : %ld (CPU@~%2.2fMhz : %d%%)", FPS, (float) (((float) IPS) / 1000000.0), (int) ((((float) IPS) / 1770000.0) * 100.0));  
       
        blit(VideoBuffer, Buffer, 0, 0, 0, 0, 256, 240);
        blit(Buffer, screen, 0, 0, 0, 0, 512+256, 512);

        return ppu_execNMIonVBlank;
    }

    if (key[KEY_B])
    {
        blit(VideoBuffer, Buffer, 0, 0, 0, 0, 256, 240);
        blit(Buffer, screen, 0, 0, 0, 0, 512 + 256, 480);
    }


    if (scanline == (239 + VBLANK_TIME))
    {
        ppu_inVBlankTime = 0;
        ppu_spriteZeroHit = 0;
        ppu_scanlineSpriteOverflow = 0;
    }
    return 0;
}

byte PPU_RegValues[8];

byte ppu_readReg(byte id)
{
    id &= 0x07;
    static byte garbage;
    static byte lastValue;
    switch(id)
    {
    default:
        garbage = PPU_RegValues[id];
        break;
    case 0x02: /* PPU Status Register */

        /* Reset VRam 2005/2006 flipflop */
        ppu_VramAccessFlipFlop = 0;
        garbage = 0;

        garbage |= (ppu_inVBlankTime!=0)          ?PPU_FLAG_SR_VBLANK:0;
        garbage |= (ppu_spriteZeroHit!=0)         ?PPU_FLAG_SR_SPRT0:0;
        garbage |= (ppu_scanlineSpriteOverflow!=0)?PPU_FLAG_SR_8SPRT:0;

        ppu_inVBlankTime = 0;
        break;

    case 0x04: /* SPR-RAM I/O */
        garbage = ppu_mem_spritesTable[ppu_mem_sptrTablePtr];
        break;

    case 0x07: /* VRAM I/O */
        if (PPU_Reg_Counter < 0x3F00)
        {
            garbage = lastValue;
            lastValue = ppu_readMemory((PPU_Reg_Counter>>8) & 0x3F,
                                       PPU_Reg_Counter & 0xFF);
        }
        else
        {
            lastValue = ppu_readMemory( 0x2F,
                                        PPU_Reg_Counter & 0xFF);
            garbage = ppu_readMemory( 0x3F,
                                      PPU_Reg_Counter & 0xFF);
        }

        PPU_Reg_Counter += ppu_addrIncrement;

        break;
    }
    return garbage;    
}


void ppu_writeReg(byte id, byte val)
{
    id &= 0x07;
    PPU_RegValues[id] = val;
    switch(id)
    {
    default:
        break;

    case 0x00: /* PPU Control Register #1 */

/*
+===============+===============================================+
|2000           |      1  0                     4               |
+---------------+-----------------------------------------------+
|               |+===++=++=++=====++=====++===++=++========++==+|
|PPU registers  || FV||V||H||   VT||   HT|| FH||S||     PAR||AR||
|PPU counters   |+---++-++-++-----++-----++===++=++========++==+|
|               |+===++=++=++=====++=====+                      |
+---------------+-----------------------------------------------+
*/
        /* Set PPU internal registers */
        PPU_Reg_V = (val & 0x02)?1:0;
        PPU_Reg_H = (val & 0x01)?1:0;
        PPU_Reg_S = (val & 0x10)?0x1000:0x0000;

        /* Set Other parameters */
        ppu_addrIncrement      = (val & 0x04)?0x20:0x01;
        ppu_spritePatternTable = (val & 0x08)?0x1000:0;
        ppu_spriteSize         = (val & 0x20)?16:8;
        ppu_execNMIonVBlank    = (val & 0x80)?1:0;
        break;

    case 0x01: /* PPU Control Register #2 */

        ppu_spriteVisibility     = (val & 0x10)?1:0;
        ppu_backgroundVisibility = (val & 0x08)?1:0;
        ppu_spriteClipping       = (val & 0x04)?1:0;
        ppu_backgroundClipping   = (val & 0x02)?1:0;
        ppu_displayType          = (val & 0x01)?1:0;

        ppu_updateSpriteScanlineTable();
        break;

    case 0x03: /* SPR-RAM Address Register */
        ppu_mem_sptrTablePtr = val;
        break;

    case 0x04: /* SPR-RAM I/O */
        ppu_mem_spritesTable[ppu_mem_sptrTablePtr++] = val;
        ppu_updateSpriteScanlineTable();
        break;

    case 0x05: /* 2005 VRAM Register */
/*
+===============+===============================================+
|2005/1         |                   76543  210                  |
|2005/2         | 210        76543                              |
+---------------+-----------------------------------------------+
|               |+===++=++=++=====++=====++===++=++========++==+|
|PPU registers  || FV||V||H||   VT||   HT|| FH||S||     PAR||AR||
|PPU counters   |+---++-++-++-----++-----++===++=++========++==+|
|               |+===++=++=++=====++=====+                      |
+---------------+-----------------------------------------------+
*/
        if (ppu_VramAccessFlipFlop == 0)
        {
            ppu_VramAccessFlipFlop = ~0;

            PPU_Reg_FH =  val & 0x07;
            PPU_Reg_HT = (val & 0xF8) >> 3;
        }
        else
        {
            ppu_VramAccessFlipFlop = 0;

            PPU_Reg_FV =  val & 0x07;
            PPU_Reg_VT = (val & 0xF8) >> 3;
        }

        break;

    case 0x06: /* 2006 VRAM Register */
/*
+===============+===============================================+
|2006/1         | -54  3  2  10                                 |
|2006/2         |              765  43210                       |
+---------------+-----------------------------------------------+
|               |+===++=++=++=====++=====++===++=++========++==+|
|PPU registers  || FV||V||H||   VT||   HT|| FH||S||     PAR||AR||
|PPU counters   |+---++-++-++-----++-----++===++=++========++==+|
|               |+===++=++=++=====++=====+                      |
+---------------+-----------------------------------------------+
*/
        if (ppu_VramAccessFlipFlop == 0)
        {
            ppu_VramAccessFlipFlop = ~0;

            PPU_Reg_FV = (val >> 4) & 0x03;
            PPU_Reg_V  = (val >> 3) & 0x01;
            PPU_Reg_H  = (val >> 2) & 0x01;
            PPU_Reg_VT =  (PPU_Reg_VT & 0x07) | ((val & 0x03) << 3);
        }
        else
        {
            ppu_VramAccessFlipFlop = 0;
            PPU_Reg_VT = (PPU_Reg_VT & 0x18) | ((val >> 5) & 0x07);
            PPU_Reg_HT =  val & 0x1F;

            ppu_updateCounters();
        }

        break;

    case 0x07: /* VRAM I/O */
/*
+---------------+-----------------------------------------------+
|               |+===++=++=++=====++=====++===++=++========++==+|
|PPU registers  || FV||V||H||   VT||   HT|| FH||S||     PAR||AR||
|PPU counters   |+---++-++-++-----++-----++===++=++========++==+|
|               |+===++=++=++=====++=====+                      |
+---------------+-----------------------------------------------+
|2007 access    |  DC  B  A  98765  43210                       |
+===============+===============================================+
*/

        ppu_writeMemory((PPU_Reg_Counter>>8) & 0x3F, PPU_Reg_Counter & 0xFF, val);
        PPU_Reg_Counter += ppu_addrIncrement;

        break;
    }
}

void ppu_fillSprRamDMA(byte value)
{
    short i;
    byte *ptr = get_page_ptr(value);
    for (i = 0; i < 0x100; i++)
    {
        ppu_mem_spritesTable[(ppu_mem_sptrTablePtr + i)&0xFF] = *(ptr+i);
    }
    ppu_updateSpriteScanlineTable();
}
