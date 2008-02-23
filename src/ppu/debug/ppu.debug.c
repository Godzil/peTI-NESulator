/*
 *  PPU Debug utilities - The TI-NESulator Project
 *  ppu.debug.c
 *
 *  Created by Manoel Trapier.
 *  Copyright 2003-2008 986 Corp. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <allegro.h>

#define __TINES_PPU_INTERNAL__

#include <ppu/ppu.h>
#include <ppu/ppu.memory.h>
#include <ppu/ppu.debug.h>

#include <types.h>

extern BITMAP *Buffer;

extern unsigned short ppu_spritePatternTable;

extern short PPU_Reg_S;

void DebugColor()
{
#ifdef TO_MAKE
  static unsigned short x = 128;
  static unsigned short y = 128;
  unsigned char OldDisplayPalette = ppu.DisplayPalette;
  byte keyb;
  unsigned int i;
  unsigned long Color;

  NOBLIT = 1;

  ppu.DisplayPalette = ~0;

  while(!key[KEY_ESC])
  {
    frame++;
    PPUVBlank();

    Color = /*Buffer->line[y][x]*/ _getpixel(Buffer, x, y);

    textprintf(Buffer, font, 5, 340, GetColor(3), "Pos [%d:%d] Color: %d Bg: %d", x, y, Color, BgColor);

    line(Buffer, x-10, y, x+10, y, GetColor(1));
    line(Buffer, x, y-10, x, y+10, GetColor(1));


    /*
    rect(Buffer, 0, 255, 4 * 20 + 2, 255 + 4 * 20 + 2, 0);
    rect(Buffer, 90, 255, 90 + 4 * 20 + 2, 255 + 4 * 20 + 2, 0);
    for (i = 0; i < 16; i++)
    {
      rectfill(Buffer, 1 + (i % 4) * 20, 256 + (i / 4) * 20, 1 + (i % 4) * 20 + 20, 256 + (i / 4) * 20 + 20, PPU_Rd(0x3F00 + i]);
      rectfill(Buffer, 91 + (i % 4) * 20, 256 + (i / 4) * 20, 91 + (i % 4) * 20 + 20, 256 + (i / 4) * 20 + 20, PPU_Rd(0x3F10 + i]);
    }*/
    for( i = 0; i < 16; i++)
    {
      if (GetColor(PPU_Rd(0x3F00 + i]) == Color)
      {
    line(Buffer, 1+(i%4)*20, 256 + (i / 4) * 20, 1 + (i % 4) * 20 + 20, 256 + (i / 4) * 20 + 20, 0xFFFFFFFF); 
    line(Buffer, 1+(i%4)*20, 256 + (i / 4) * 20 + 20, 1 + (i % 4) * 20 + 20, 256 + (i / 4) * 20, 0xFFFFFFFF); 
      }

      if (GetColor(PPU_Rd(0x3F10 + i]) == Color)
      {
    line(Buffer, 91+(i%4)*20, 256 + (i / 4) * 20, 91 + (i % 4) * 20 + 20, 256 + (i / 4) * 20 + 20, 0xFFFFFFFF); 
    line(Buffer, 91+(i%4)*20, 256 + (i / 4) * 20 + 20, 91 + (i % 4) * 20 + 20, 256 + (i / 4) * 20, 0xFFFFFFFF); 
      }
       
    }

    blit(Buffer, screen, 0, 0, 0, 0, 512 + 256, 480);    

    if (keypressed())
    {
      keyb = (readkey() & 0xFF);

      if (keyb == '4')
      {
    x--;
      }
      if (keyb == '8')
      {
    y--;
      }
      if (keyb == '6')
      {
    x++;
      }
      if (keyb == '2')
      {
    y++;
      }


    }
  }
  ppu.DisplayPalette = OldDisplayPalette;
  NOBLIT = 0;
  
#endif
}

void DebugSprites()
{
#ifdef TO_MAKE
  byte keyb;
  static int SelSprite = 0;
  PPUSprite sprite;
  NOBLIT = 1;
  ppu.ControlRegister2.b |= PPU_CR2_SPRTVISIBILITY;

  while(!key[KEY_ESC])
  {
    frame++;
    PPUVBlank();
    sprite = PPUGetSprite(SelSprite);

    if (ppu.ControlRegister1.b & PPU_CR1_SPRTSIZE)
    {
      rect(Buffer, sprite.x-1, sprite.y-1, sprite.x+9, sprite.y+17, 1);
    }
    else
    {
      rect(Buffer, sprite.x-1, sprite.y-1, sprite.x+9, sprite.y+9, 1);
    }

    textprintf(Buffer, font, 5, 340, GetColor(3), "Sprite %d [%d:%d]", SelSprite, sprite.x, sprite.y);
    textprintf(Buffer, font, 5, 349, GetColor(3), "B0: 0x%X  B1: 0x%X  B2: 0x%X  B3: 0x%X",sprite.y,sprite.tileid,sprite.flags.b,sprite.x);
    textprintf(Buffer, font, 5, 358, GetColor(3), "Tile Index: %d", sprite.tileid);
    textprintf(Buffer, font, 5, 367, GetColor(3), "Vertical Flip: %d", sprite.flags.s.VFlip);
    textprintf(Buffer, font, 5, 376, GetColor(3), "Horizontal Flip: %d", sprite.flags.s.HFlip);
    textprintf(Buffer, font, 5, 385, GetColor(3), "Background Priority: %d", sprite.flags.s.BgPrio);
    textprintf(Buffer, font, 5, 394, GetColor(3), "Upper Color: %d", sprite.flags.s.UpperColor);

    blit(Buffer, screen, 0, 0, 0, 0, 512 + 256, 480);    

    if (keypressed())
    {
      keyb = (readkey() & 0xFF);
      if (keyb == '+')
    SelSprite = (SelSprite<63)?SelSprite+1:0;
      if (keyb == '-')
    SelSprite = (SelSprite>0)?SelSprite-1:63;

      if (keyb == 'h')
      {
    sprite.flags.s.HFlip = ~sprite.flags.s.HFlip;
    PPUSetSprite(SelSprite, &sprite);
      }
      if (keyb == 'b')
      {
    sprite.flags.s.BgPrio = ~sprite.flags.s.BgPrio;
    PPUSetSprite(SelSprite, &sprite);
      }
      if (keyb == 'v')
      {
    sprite.flags.s.VFlip = ~sprite.flags.s.VFlip;
    PPUSetSprite(SelSprite, &sprite);
      }

      if (keyb == '4')
      {
    sprite.x--;
    PPUSetSprite(SelSprite, &sprite);
      }
      if (keyb == '8')
      {
    sprite.y--;
    PPUSetSprite(SelSprite, &sprite);
      }
      if (keyb == '6')
      {
    sprite.x++;
    PPUSetSprite(SelSprite, &sprite);
      }
      if (keyb == '2')
      {
    sprite.y++;
    PPUSetSprite(SelSprite, &sprite);
      }
    }
  }
  NOBLIT = 0;
#endif
}

#define GetTilePos(addr,x,y) (addr+x+(y*32))

#define GetTileColor(tile,x1,y1)	( ( ppu_readMemory(((tile+y1)>>8)&0xFF, (tile+y1) & 0xFF) & (1<<(7-x1)) ) == 0 ? 0 : 1 ) | \
									( ( ppu_readMemory(((tile+y1+8)>>8) & 0xFF, (tile+y1+8) &0xFF) & (1<<(7-x1)) ) == 0 ? 0 : 1<<1 )

#define PPU_Rd(addr) ppu_readMemory((addr>>8)&0xFF, addr&0xFF)									

void ppu_dumpOneNameTable(unsigned short nametable, int xd, int yd)
{
    byte x,y, x1, y1, Color;
    unsigned short TileID;
    
    for (x = 0; x < 32; x++)
        for (y = 0; y < 30; y++)
        {
            TileID = (PPU_Rd(0x2000 + nametable + x + (y * 32)) << 4) | (PPU_Reg_S);

            for (x1 = 0; x1 < 8; x1++)
                for (y1 = 0; y1 < 8; y1++)
                {
                    Color = GetTileColor(TileID, x1, y1);
//                    if (ppu.DisplayAttributeTable != 0)
//                        Color |= (Buffer->line[(8 * y) + 240 + y1][(8 * x) + 256 + x1] & 0x3) << 2;
                    Color += (nametable>>8);
                    Color = ppu_readMemory(0x3F, Color);
                    
                    
                    _putpixel(Buffer, (8 * x) + xd + x1, (8 * y) + yd + y1, Color);
                }
         }
}

void ppu_dumpOneAttributeTable(unsigned short nametable, int xd, int yd)
{
    int x, x1, y1, Color, AttrByte;
    for (x = 0; x < 0x40; x++)
    {
        AttrByte = PPU_Rd(nametable + 0x23C0 + x);
        x1 = x % 8;
        y1 = x / 8;
    
        Color = AttrByte & 0x3; // Pattern 1;
//        Color = PPU_Rd(0x3F00 + (Color * 4) + 1);
        rectfill(Buffer,xd+(x1*32),yd+(y1*32),xd+15+(x1*32),yd+15+(y1*32),Color);
        
        textprintf_ex(Buffer, font, 4+xd+(x1*32), 4+yd+(y1*32), ~Color, Color, "%X", Color);

            
        Color = (AttrByte>>2) & 0x3; // Pattern 2;
//        Color = PPU_Rd(0x3F00 + (Color * 4) + 1);          
        rectfill(Buffer,16+xd+(x1*32),yd+(y1*32),16+xd+15+(x1*32),yd+15+(y1*32),Color);

        textprintf_ex(Buffer, font, 4+xd+(x1*32)+16, 4+yd+(y1*32), ~Color, Color, "%X", Color);
            
        Color = (AttrByte>>4) & 0x3; // Pattern 3;
//        Color = PPU_Rd(0x3F00 + (Color * 4) + 1);
        rectfill(Buffer,xd+(x1*32),16+yd+(y1*32),xd+15+(x1*32),16+yd+15+(y1*32),Color);

        textprintf_ex(Buffer, font, 4+xd+(x1*32), 4+yd+(y1*32)+16, ~Color, Color, "%X", Color);
            
        Color = (AttrByte>>6) & 0x3; // Pattern 4;
//        Color = PPU_Rd(0x3F00 + (Color * 4) + 1);
        rectfill(Buffer,16+xd+(x1*32),16+yd+(y1*32),16+xd+15+(x1*32),16+yd+15+(y1*32),Color);

        textprintf_ex(Buffer, font, 4+xd+(x1*32)+16, 4+yd+(y1*32)+16, ~Color, Color, "%X", Color);
        
        rect(Buffer, xd+(x1*32), yd+(y1*32), xd+(x1*32) + 31 , yd+(y1*32) + 31, 1);
        line(Buffer, xd+(x1*32)+16, yd+(y1*32), xd+(x1*32) + 16 , yd+(y1*32) + 31, 5);
        line(Buffer, xd+(x1*32), yd+(y1*32)+16, xd+(x1*32) + 31 , yd+(y1*32) + 16, 5);
        
    }
}
extern byte *ppu_mem_nameTables;
extern int ppu_screenMode;
void ppu_dumpNameTable(int xd, int yd)
{
   // ppu_setPagePtr1k(0x20, ppu_mem_nameTables + 0x000);
   // ppu_setPagePtr1k(0x24, ppu_mem_nameTables + 0x400);
   // ppu_setPagePtr1k(0x28, ppu_mem_nameTables + 0x800);
   // ppu_setPagePtr1k(0x2C, ppu_mem_nameTables + 0xC00);
    
    ppu_dumpOneNameTable(0x800, xd      , yd);
    ppu_dumpOneNameTable(0xC00, xd + 256, yd);
    ppu_dumpOneNameTable(0x000, xd      , yd + 240);
    ppu_dumpOneNameTable(0x400, xd + 256, yd + 240);
    
   // ppu_setScreenMode(ppu_screenMode);
    
}

void ppu_dumpAttributeTable(int xd, int yd)
{
    ppu_dumpOneAttributeTable(0x800, xd      , yd);
    ppu_dumpOneAttributeTable(0xC00, xd + 256, yd);
    ppu_dumpOneAttributeTable(0x000, xd      , yd + 240);
    ppu_dumpOneAttributeTable(0x400, xd + 256, yd + 240);
}

void ppu_dumpPattern(int xd, int yd)
{
    int x1, y1, x, y, Color, i, TileID;
/* y:346 */
        x1 = 0;
        y1 = 0;
        for (i = 0; i < 256; i++)
        {
            TileID = 0x0000 + (i << 4);
            for (x = 0; x < 8; x++)
                for (y = 0; y < 8; y++)
                {
                    Color = GetTileColor(TileID, x, y);

                    _putpixel(Buffer, 10 + x1 + x, 347 + y1 + y, /*ppu_readMemory(0x3F,*/ Color/*)*/);
                }

            x1 += 8;
            if (x1 >= 128)
            {
                x1 = 0;
                y1 += 8;
            }
        }
        x1 = 0;
        y1 = 0;
        for (i = 0; i < 256; i++)
        {
            TileID = 0x1000 + (i << 4);

            for (x = 0; x < 8; x++)
                for (y = 0; y < 8; y++)
                {

                    Color = GetTileColor(TileID, x, y);

                    _putpixel(Buffer, 10 + 128 + x1 + x, 347 + y1 + y, /*ppu_readMemory(0x3F,*/ 0x10 | Color/*)*/ );

                }
            x1 += 8;
            if (x1 >= 128)
            {
                x1 = 0;
                y1 += 8;
            }
        }
}
									
void ppu_dumpPalette(int x, int y)
{
    int i;

    textout(Buffer, font, "Bg Palette", x , y, 5);
    textout(Buffer, font, "Sprt Palette", x + 90, y, 5);

    rect(Buffer, x+0, y+20, x+4 * 20 + 2, y + 4 * 20 + 22, 0);
    rect(Buffer, x+90, y+20, x+90 + 4 * 20 + 2, y + 4 * 20 + 22, 0);

    for (i = 0; i < 16; i++)
    {
        rectfill(Buffer, x + 1 + (i % 4) * 20, y + 21 + (i / 4) * 20, x + 1 + (i % 4) * 20 + 20, y + 21 + (i / 4) * 20 + 20, ppu_readMemory(0x3F, i));
        rectfill(Buffer, x + 91 + (i % 4) * 20, y + 21 +(i / 4) * 20, x + 91 + (i % 4) * 20 + 20, y + 21 +(i / 4) * 20 + 20, ppu_readMemory(0x3F, i+0x10));
    }
}
