/*
 *  PPU emulation - The peTI-NESulator Project
 *  ppu.c
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <os_dependent.h>

#define __TINES_PPU_INTERNAL__

#include <ppu/ppu.h>
#include <ppu/ppu.memory.h>
#include <ppu/ppu.debug.h>

#include <memory/manager.h>

#include <os_dependent.h>

#define __TINES_PLUGINS__

#include <plugins/manager.h>

extern int VBLANK_TIME;

extern volatile int frame;
extern volatile unsigned long IPS, FPS;

extern unsigned long ColorPalette[9 * 63];
extern short IRQScanHit;
extern short SZHit;

typedef struct spriteData
{
   uint8_t palette;
   uint8_t flip_h;
   uint8_t flip_v;
   uint8_t priority;
   uint8_t tile;
   uint8_t bank;
   uint8_t y;
   uint8_t x;
   uint8_t rel_y;
   uint8_t inUse;
} spriteData;


/* PPU registers */

/* NT: Name Table */
uint8_t PPU_Reg_NT;

/* AT: Attribute/Color Table */
uint8_t PPU_Reg_AT;

/* FV: Fine Vertical Scroll latch/counter */
uint8_t PPU_Reg_FV;

/* HV: Fine Horizontal Scroll latch/counter */
uint8_t PPU_Reg_FH;

/* VT: Vertical Tile indev latch/counter */
uint8_t PPU_Reg_VT;

/* HT: Horizontal Tile indev latch/counter */
uint8_t PPU_Reg_HT;

/* V: Vertical Name Table Selection latch/counter */
uint8_t PPU_Reg_V;

/* H: Horizontal Name Table Selection latch/counter */
uint8_t PPU_Reg_H;

/* S: Playfield pattern table selection latch */
unsigned short PPU_Reg_S;

/* PAR: Picture Address Register */
uint8_t PPU_Reg_PAR;

/* AR: Tile Attribute (palette select) value latch */
uint8_t PPU_Reg_AR;

unsigned short PPU_Reg_Counter;


/* PPU Memory Areas */
uint8_t *ppu_mem_nameTables;
uint8_t *ppu_mem_patternTables;
uint8_t *ppu_mem_paletteValues;

uint8_t ppu_mem_spritesTable[0x100];
uint8_t ppu_mem_sptrTablePtr;


/* Some other PPU "registers" */
uint8_t ppu_VramAccessFlipFlop;

uint8_t ppu_inVBlankTime;
uint8_t ppu_spriteZeroHit;
uint8_t ppu_scanlineSpriteOverflow;

uint8_t ppu_bgColor;

/* CR #1 variables */
uint16_t ppu_spritePatternTable;
uint8_t ppu_spriteSize;
uint8_t ppu_addrIncrement;
uint8_t ppu_execNMIonVBlank;

/* CR #2 variables */
uint8_t ppu_spriteVisibility;
uint8_t ppu_backgroundVisibility;
uint8_t ppu_spriteClipping;
uint8_t ppu_backgroundClipping;
uint8_t ppu_displayType;

uint8_t ppu_mirrorMode;
uint8_t ppu_singleScreenMode;
uint8_t ppu_screenMode;

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

   /*uint8_t defaultColors[] = { 0x09,0x01,0x00,0x01,0x00,0x02,0x02,0x0D,0x08,0x10,0x08,0x24,0x00,0x00,0x04,0x2C,
                            0x09,0x01,0x34,0x03,0x00,0x04,0x00,0x14,0x08,0x3A,0x00,0x02,0x00,0x20,0x2C,0x08 };*/

   if ( ppu_initMemory() )
      return -1;

   /* Set ppu memory parameters */

   /* First: Allocate each memory zone */
   ppu_mem_patternTables = (uint8_t *) malloc(PPU_MEM_PATTERNTABLES_SIZE);
   if ( !ppu_mem_patternTables )
      return -1;

   ppu_mem_nameTables = (uint8_t *) malloc(PPU_MEM_NAMETABLE_SIZE);
   if ( !ppu_mem_nameTables )
      return -1;

   ppu_mem_paletteValues = (uint8_t *) malloc(PPU_MEM_PALETTEVALUES_SIZE);
   if ( !ppu_mem_paletteValues )
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
   ppu_setPagePtr(0x3F, ppu_mem_paletteValues);

   for ( i = 0x00 ; i < 0x0F ; i++ )
      ppu_setPageGhost(0x30 + i, true, 0x20 + i);

   /* Third: set registers to defaults */

   /* Now test the memory ! */

   /* Fille PPU memory with garbage */
   for ( i = 0x0000 ; i < 0x2000 ; i++ )
      ppu_mem_patternTables[i] = rand() % 0xFF;
   for ( i = 0x0000 ; i < 0x1000 ; i++ )
      ppu_mem_nameTables[i] = rand() % 0xFF;
   for ( i = 0x0000 ; i < 0x001F ; i++ )
      ppu_mem_paletteValues[i] = rand() % 0xFF;

   //memcpy(ppu_mem_paletteValues, defaultColors, 32);

   /* Set some other variables */
   ppu_VramAccessFlipFlop = 0;

   ppu_addrIncrement = 1;
   ppu_spritePatternTable = 0;
   ppu_spriteSize = 8;
   ppu_execNMIonVBlank = 0;

   ppu_spriteVisibility = 0;
   ppu_backgroundVisibility = 0;
   ppu_spriteClipping = 0;
   ppu_backgroundClipping = 0;
   ppu_displayType = 0;

   ppu_inVBlankTime = 0;
   ppu_bgColor = 0;

   /* Set PPU registers on CPU side */
   set_page_rd_hook(0x20, ppu_readReg);
   set_page_wr_hook(0x20, ppu_writeReg);

   set_page_readable(0x20, true);
   set_page_writeable(0x20, true);


   /* Set PPU Ghost Registers */
   for ( i = 0x21 ; i < 0x40 ; i++ )
      set_page_ghost(i, true, 0x20);

   /* allocate the PPU Video memory */
   graphics_init();

   return 0;
}

void ppu_setMirroring(uint8_t direction)
{
   if ( ppu_screenMode != PPU_SCMODE_NORMAL )
      return;

   if ( ppu_mirrorMode == direction )
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

void ppu_setSingleScreen(uint8_t screen)
{
   if ( ppu_screenMode != PPU_SCMODE_SINGLE )
      return;
   if ( ppu_singleScreenMode == screen )
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
void ppu_setScreenMode(uint8_t mode)
{
   if ( ppu_screenMode == mode )
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
   PPU_Reg_Counter = ( PPU_Reg_FV & 0x07 ) << 12;
   PPU_Reg_Counter |= PPU_Reg_V << 11;
   PPU_Reg_Counter |= PPU_Reg_H << 10;
   PPU_Reg_Counter |= PPU_Reg_VT << 5;
   PPU_Reg_Counter |= PPU_Reg_HT;
}

int ppu_hblank(uint16_t scanline)
{
   uint32_t j;

   /* Sprite to display on current scanline */
   spriteData scanSprites[8];
   ppu_scanlineSpriteOverflow = 0;

   if ( scanline == 0 )
   {
      if ( ( ppu_spriteVisibility != 0 ) || ( ppu_backgroundVisibility != 0 ) )
         ppu_updateCounters();
   }

   if ( scanline < 240 )
   {
      int i;
      uint8_t  spriteCount = 0;

      /* Search for sprites on current scanline */
      for (i = 0 ; i < 8 ; i++)
      {
         scanSprites[i].inUse = 0;
      }

      for (i = 0 ; i < 64 && spriteCount < 8; i++)
      {
         uint8_t spriteY = ppu_mem_spritesTable[i*4] + 1;
         if ((scanline >= spriteY) && (scanline < (spriteY + ppu_spriteSize)))
         {
            /* This sprite is on the scanline */
            scanSprites[spriteCount].inUse = 1;

            scanSprites[spriteCount].x = ppu_mem_spritesTable[i*4 + 3];
            scanSprites[spriteCount].y = spriteY;
            scanSprites[spriteCount].rel_y = scanline - scanSprites[spriteCount].y;

            scanSprites[spriteCount].tile = ppu_mem_spritesTable[i * 4 + 1];
            scanSprites[spriteCount].bank = ppu_mem_spritesTable[i * 4 + 1] & 0x01;
            scanSprites[spriteCount].flip_h = ( ppu_mem_spritesTable[i * 4 + 2] & PPU_SPRITE_FLAGS_HFLIP )?1:0;
            scanSprites[spriteCount].flip_v = ( ppu_mem_spritesTable[i * 4 + 2] & PPU_SPRITE_FLAGS_VFLIP )?1:0;
            scanSprites[spriteCount].palette = ( ppu_mem_spritesTable[i * 4 + 2] & PPU_SPRITE_FLAGS_UPPERCOLOR ) & 0x0F;
            scanSprites[spriteCount].priority = ( ppu_mem_spritesTable[i * 4 + 2] & PPU_SPRITE_FLAGS_BGPRIO )?1:0;

            spriteCount++;
         }
      }


      ppu_bgColor = ppu_readMemory(0x3F, 00);
      /* For each PPU pixel of this scanline */
      for ( i = 0 ; i < 256 ; i++ )
      {
         uint16_t addr;
         uint8_t value;

         uint8_t pixelColor = 0;
         uint8_t BgColor = 0;

         /* Set the current pixel color to the bg color */
         pixelColor = ppu_bgColor;
         BgColor = 0;

         /* Compute current pixel bg color if bg is visible */
         if (( ppu_backgroundVisibility == 1 ) && (!getKeyStatus('B')))
         {
            if ( ((i < 8) && (!ppu_backgroundClipping)) || (i >= 8))
            {
               addr = ( PPU_Reg_Counter & 0x0C00 );
               addr = addr | 0x03C0;
               addr |= ( PPU_Reg_Counter >> 4 ) & 0x0038;
               addr |= ( PPU_Reg_Counter >> 2 ) & 0x0007;

               PPU_Reg_AR = ppu_readMemory(0x20 | ( ( addr >> 8 ) & 0x0F ), addr & 0xFF);

               PPU_Reg_AR = PPU_Reg_AR >> ( ( ( PPU_Reg_Counter >> 4 ) & 0x04 ) | ( ( PPU_Reg_Counter ) & 0x02 ) );
               PPU_Reg_AR = ( PPU_Reg_AR << 2 ) & 0x0C;

               PPU_Reg_PAR = ppu_readMemory(0x20 | ( ( PPU_Reg_Counter >> 8 ) & 0x0F ), PPU_Reg_Counter & 0xFF);

               addr = PPU_Reg_S;
               addr |= ( ( PPU_Reg_PAR & 0xFF ) << 4 );
               addr |= ( ( PPU_Reg_Counter >> 12 ) & 0x07 );

               value = ppu_readMemory(( addr >> 8 ), addr);
               BgColor = ( value & ( 1 << ( 7 - ( i + PPU_Reg_FH ) % 8 ) ) )?0x01:0;

               value = ppu_readMemory(( addr >> 8 ), addr | 0x08);
               BgColor |= ( value & ( 1 << ( 7 - ( i + PPU_Reg_FH ) % 8 ) ) )?0x02:0;

               if ( BgColor > 0x00 )
               {
                  BgColor |= PPU_Reg_AR;
                  BgColor &= 0x0F;

                  pixelColor = ppu_readMemory(0x3F, BgColor);
               }

               if ( ( ( i + PPU_Reg_FH ) % 8 ) == 7 )
               {
                  uint16_t tmp_HHT = 0;

                  tmp_HHT = ( ( PPU_Reg_Counter >> 5 ) & 0x0020 ) |
                            ( PPU_Reg_Counter & 0x001F );
                  tmp_HHT = ( tmp_HHT + 1 ) & 0x003F;

                  /* Reassemble with HT & H */
                  PPU_Reg_Counter = ( PPU_Reg_Counter & 0xFBE0 ) |
                                    ( ( tmp_HHT & 0x0020 ) << 5 ) |
                                    ( tmp_HHT & 0x001F );
               }
            }
#if 0
            // ISPAL
            else
            {
               pixelColor = 0x1D;
            }
#endif
         }

         /* Now calculate if there is a sprite here and sprite visibility is on */
         if ( ppu_spriteVisibility == 1 )
         {
            if (((!ppu_spriteClipping) && (i < 8)) || (i >= 8))
            {
               for( j = 0 ; j < 8 ; j++)
               {
                  spriteData *sprite = &scanSprites[j];
                  int8_t spriteRelX;

                  if ( sprite->inUse == 0 )
                     break;

                  spriteRelX = i - sprite->x;

                  if ( ( spriteRelX >= 0 ) && ( spriteRelX < 8 ) )
                  {
                     uint8_t SpriteColor = 0;

                     /* Get sprite tile address */
                     if ( ppu_spriteSize == 8 )
                     {
                        addr = ( sprite->tile << 4 ) + ppu_spritePatternTable;
                     }
                     else
                     {
                        if ( sprite->rel_y < 8 )
                        {
                           addr = ( ( ( sprite->tile & 0xFE ) + ( sprite->flip_v?1:0 ) ) << 4 ) +
                                  ( ( sprite->bank )?0x1000:0x0000 );
                        }
                        else
                        {
                           addr = ( ( ( sprite->tile & 0xFE ) + ( sprite->flip_v?0:1 ) ) << 4 ) +
                                  ( ( sprite->bank )?0x1000:0x0000 );
                        }
                     }

                     if ( sprite->flip_v )
                     {
                        addr += 7;
                        addr -= sprite->rel_y % 8;
                     }
                     else
                     {
                        addr += sprite->rel_y % 8;
                     }

                     if ( sprite->flip_h )
                     {
                        value = ppu_readMemory(( addr >> 8 ), addr);
                        SpriteColor = ( value & ( 1 << ( spriteRelX ) ) )?0x01:0;

                        value = ppu_readMemory(( addr >> 8 ), addr | 0x08);
                        SpriteColor |= ( value & ( 1 << ( spriteRelX ) ) )?0x02:0;
                     }
                     else
                     {
                        value = ppu_readMemory(( addr >> 8 ), addr);
                        SpriteColor = ( value & ( 1 << ( 7 - ( spriteRelX ) ) ) )?0x01:0;
                        value = ppu_readMemory(( addr >> 8 ), addr | 0x08);
                        SpriteColor |= ( value & ( 1 << ( 7 - ( spriteRelX ) ) ) )?0x02:0;
                     }

                     /* If we get a color different from 0, the pixel is not transparent */
                     if ( SpriteColor > 0 )
                     {
                        /* Add second part of the colour */
                        SpriteColor |= ( ( sprite->palette ) << 2 );
                        SpriteColor &= 0x0F;

                        if ( j == 0 )
                        {
                           /* Sprite 0 */
                           if ( ( BgColor != 0 ) && ( !ppu_spriteZeroHit ) )
                           {
                              ppu_spriteZeroHit = ( ppu_backgroundVisibility )?1:0;
                              if ( ppu_spriteZeroHit )
                                 SZHit = scanline;
                           }
                        }

                        if ( sprite->priority )
                        {
                           if ( SpriteColor > 0x00 )
                           {
                              if ( BgColor == 0 )
                              {
                                 pixelColor = ppu_readMemory(0x3F, ( 0x10 + SpriteColor ));
                              }
                              break;
                           }
                        }
                        else
                        {
                           if ( SpriteColor != 0x00 )
                           {
                              pixelColor = ppu_readMemory(0x3F, ( 0x10 + SpriteColor ));
                              break;
                           }
                        }
                     }
                  }
               }

            }
         }

         /* Set to monochrome if needed */
         if ( ppu_displayType )
            pixelColor &= 0x30;

         /* draw the pixel */
         graphics_drawpixel(i, scanline, pixelColor);
      }

      if (spriteCount > 8)
      {
         ppu_scanlineSpriteOverflow = 1;
      }

      if ( ppu_backgroundVisibility == 1 )
      {
         uint16_t tmp_VVTFV = 0;

         tmp_VVTFV = ( ( PPU_Reg_Counter >> 3 ) & 0x0100 ) | /* V */
                     ( ( PPU_Reg_Counter >> 2 ) & 0x00F8 ) | /* VT */
                     ( ( PPU_Reg_Counter >> 12 ) & 0x0007 );  /* FV */

         tmp_VVTFV++;
         if ( ( tmp_VVTFV & 0x0F8 ) == 0xF0 )
         {
            tmp_VVTFV &= ~0x0F8;
            tmp_VVTFV ^= 0x100;
         }

         PPU_Reg_Counter = ( PPU_Reg_Counter & 0x041F ) |
                           ( ( tmp_VVTFV & 0x0100 ) << 3 ) | /* V */
                           ( ( tmp_VVTFV & 0x00F8 ) << 2 ) | /* VT */
                           ( ( tmp_VVTFV & 0x0007 ) << 12 );  /* FV */

         /* Update H & HT */
         PPU_Reg_Counter = ( PPU_Reg_Counter & ~0x041F ) |
                           ( PPU_Reg_H << 10 ) |
                           PPU_Reg_HT;
      }
   }
   /* Increment only V, VT & FV*/
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
   if ( scanline == 239 )
   {
      ppu_inVBlankTime = 1;
      //  textprintf_ex(Buffer, font, 260, 3, 4, 0, "FPS : %ld (CPU@~%2.2fMhz : %d%%)", FPS, (float) (((float) IPS) / 1000000.0), (int) ((((float) IPS) / 1770000.0) * 100.0));

      graphics_blit(0, 0, 256, 240);
      return ppu_execNMIonVBlank;
   }

   /* Debug tools */
   /*if ( scanline == SZHit )
   {
      graphics_drawline(0, scanline, 256, scanline, 0x12);
   }
   if ( scanline == IRQScanHit )
   {
      graphics_drawline(0, scanline, 256, scanline, 0x13);
   }*/
   /* */

   if ( scanline == ( 239 + VBLANK_TIME ) + 0 )
   {
      ppu_inVBlankTime = 0;
      ppu_spriteZeroHit = 0;
      ppu_scanlineSpriteOverflow = 0;
   }
   return 0;
}

uint8_t PPU_RegValues[8];

uint8_t ppu_readReg(uint8_t id)
{
   id &= 0x07;
   static uint8_t garbage;
   static uint8_t lastValue;
   switch(id)
   {
   default:
      garbage = PPU_RegValues[id];
      break;
   case 0x02: /* PPU Status Register */

      /* Reset VRam 2005/2006 flipflop */
      ppu_VramAccessFlipFlop = 0;
      garbage = 0;

      garbage |= ( ppu_inVBlankTime != 0 )?PPU_FLAG_SR_VBLANK:0;
      garbage |= ( ppu_spriteZeroHit != 0 )?PPU_FLAG_SR_SPRT0:0;
      garbage |= ( ppu_scanlineSpriteOverflow != 0 )?PPU_FLAG_SR_8SPRT:0;

      ppu_inVBlankTime = 0;
      break;

   case 0x04: /* SPR-RAM I/O */
      garbage = ppu_mem_spritesTable[ppu_mem_sptrTablePtr];
      break;

   case 0x07: /* VRAM I/O */
      if ( PPU_Reg_Counter < 0x3F00 )
      {
         garbage = lastValue;
         lastValue = ppu_readMemory(( PPU_Reg_Counter >> 8 ) & 0x3F,
                                    PPU_Reg_Counter & 0xFF);
      }
      else
      {
         lastValue = ppu_readMemory(0x2F,
                                    PPU_Reg_Counter & 0xFF);
         garbage = ppu_readMemory(0x3F,
                                  PPU_Reg_Counter & 0xFF);
      }

      PPU_Reg_Counter += ppu_addrIncrement;

      break;
   }
   return garbage;
}


void ppu_writeReg(uint8_t id, uint8_t val)
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
      PPU_Reg_V = ( val & 0x02 )?1:0;
      PPU_Reg_H = ( val & 0x01 )?1:0;
      PPU_Reg_S = ( val & 0x10 )?0x1000:0x0000;

      /* Set Other parameters */
      ppu_addrIncrement = ( val & 0x04 )?0x20:0x01;
      ppu_spritePatternTable = ( val & 0x08 )?0x1000:0;
      ppu_spriteSize = ( val & 0x20 )?16:8;
      ppu_execNMIonVBlank = ( val & 0x80 )?1:0;
      break;

   case 0x01: /* PPU Control Register #2 */
      ppu_spriteVisibility = ( val & 0x10 )?1:0;
      ppu_backgroundVisibility = ( val & 0x08 )?1:0;
      ppu_spriteClipping = ( val & 0x04 )?0:1;
      ppu_backgroundClipping = ( val & 0x02 )?0:1;
      ppu_displayType = ( val & 0x01 )?1:0;
      break;

   case 0x03: /* SPR-RAM Address Register */
      ppu_mem_sptrTablePtr = val;
      break;

   case 0x04: /* SPR-RAM I/O */
      ppu_mem_spritesTable[ppu_mem_sptrTablePtr++] = val;
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
      if ( ppu_VramAccessFlipFlop == 0 )
      {
         ppu_VramAccessFlipFlop = ~0;

         PPU_Reg_FH = val & 0x07;
         PPU_Reg_HT = ( val & 0xF8 ) >> 3;
      }
      else
      {
         ppu_VramAccessFlipFlop = 0;

         PPU_Reg_FV = val & 0x07;
         PPU_Reg_VT = ( val & 0xF8 ) >> 3;
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
      if ( ppu_VramAccessFlipFlop == 0 )
      {
         ppu_VramAccessFlipFlop = ~0;

         PPU_Reg_FV = ( val >> 4 ) & 0x03;
         PPU_Reg_V = ( val >> 3 ) & 0x01;
         PPU_Reg_H = ( val >> 2 ) & 0x01;
         PPU_Reg_VT = ( PPU_Reg_VT & 0x07 ) | ( ( val & 0x03 ) << 3 );
      }
      else
      {
         ppu_VramAccessFlipFlop = 0;
         PPU_Reg_VT = ( PPU_Reg_VT & 0x18 ) | ( ( val >> 5 ) & 0x07 );
         PPU_Reg_HT = val & 0x1F;

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

      ppu_writeMemory(( PPU_Reg_Counter >> 8 ) & 0x3F, PPU_Reg_Counter & 0xFF, val);
      PPU_Reg_Counter += ppu_addrIncrement;

      break;
   }
}

void ppu_fillSprRamDMA(uint8_t value)
{
   short i;
   uint8_t *ptr = get_page_ptr(value);
   for ( i = 0 ; i < 0x100 ; i++ )
   {
      ppu_mem_spritesTable[( ppu_mem_sptrTablePtr + i ) & 0xFF] = *( ptr + i );
   }
}
