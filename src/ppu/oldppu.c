/*
 *  PPU emulation - The TI-NESulator Project
 *  oldppu.c
 *
 *  Old code for the PPU, still here for unknown purposes
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
#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>

#include "ppu.h"
#include "M6502.h"

#define GetTileColor(tile,x1,y1)    ( ( ppu.Memory[tile+y1] & (1<<(7-x1)) ) == 0 ? 0 : 1 ) | \
                                    ( ( ppu.Memory[tile+y1+8] & (1<<(7-x1)) ) == 0 ? 0 : 1<<1 )

extern PPU ppu;
extern BITMAP *Buffer;
extern unsigned short ScanLine;
unsigned char BgColor;

volatile extern unsigned long IPS, FPS;

int InitPPU(PPU * ppu)
{
    if ((ppu->Memory = (unsigned char *) malloc(0x4000)) == NULL)
        return -1;

    /* Initializing register.. */
    ppu->In_VBlank = 0;
    
    ppu->BaseOneScreen = 0x2000;
    
    ppu->ControlRegister1.b = 0;
    ppu->ControlRegister2.b = 0;
    ppu->StatusRegister.b = 0;
    ppu->SPR_RAMAddr = 0;
    ppu->VRAMAddrReg1.W = 0;
    ppu->VRAMAddrReg2.W = 0;
    ppu->VRAMAddrMode = 0;

    ppu->Bg_Pattern_Table = 0x0000;
    ppu->Name_Table_Addresse = 0x2000;
    ppu->Sprt_Pattern_Table = 0x0000;
    ppu->PPU_Inc = 1;

    ppu->MirrorDir = 0;
    ppu->ScreenType = 1;


    ppu->DisplayNameTables = ~0;
    ppu->DisplayAttributeTable = ~0;
    ppu->DisplayPalette = ~0;
    ppu->DisplayVRAM = ~0;

    return 0;
}

PPUSprite PPUGetSprite(unsigned short i)
{
PPUSprite ret;

    ret.y = ppu.SPRRAM[i * 4];
    ret.tileid = ppu.SPRRAM[i * 4 + 1];
    ret.flags.BgPrio = (ppu.SPRRAM[i * 4 + 2] & (1 << 5)) == 0 ? 0 : 1;
    ret.flags.HFlip = (ppu.SPRRAM[i * 4 + 2] & (1 << 6)) == 0 ? 0 : 1;
    ret.flags.UpperColor = ppu.SPRRAM[i * 4 + 2] & 0x03;
    ret.flags.VFlip = (ppu.SPRRAM[i * 4 + 2] & (1 << 7)) == 0 ? 0 : 1;
    ret.x = ppu.SPRRAM[i * 4 + 3];
    return ret;
}


//(Addr & 0x1FF)    /* Addr In NT */
// (Addr & 0xC00) >> 2        /* NT number */
#define GetNT(a)    ( (a&0xC00) >> 10 )
#define RelAddr(a)    (a & 0x3FF)
#define PalAddr(a) (a & 0x1F)

unsigned char PPU_Rd(unsigned short Addr)
{
    if (Addr > (unsigned short) 0x3FFF)
    {
        Addr &= 0x3FFF;
    }
    if ((Addr < 0x3F00) && (Addr >= 0x2000))
    {
        if (Addr > 0x3000)
        {
            Addr -= 0x1000;
        }
        if (ppu.ScreenType == 0)
        {        /* 1 Screen Mode */
            return ppu.Memory[RelAddr(Addr) + ppu.BaseOneScreen];
        }
        else
            if (ppu.ScreenType)
            {    /* Miroring mode */
                if (ppu.MirrorDir)
                {    /* Horizontal */
                    if (GetNT(Addr) & 0x2)
                    {    /* NT2-3 */
                        return ppu.Memory[0x2000 + RelAddr(Addr)];
                    }
                    else
                    {
                        return ppu.Memory[0x2400 + RelAddr(Addr)];
                    }
                }
                else
                {    /* Vertical */
                    if (GetNT(Addr) & 0x1)
                    {    /* NT0-2 */
                        return ppu.Memory[0x2400 + RelAddr(Addr)];
                    }
                    else
                    {
                        return ppu.Memory[0x2000 + RelAddr(Addr)];
                    }
                }
            }
            else
            {    /* Four Screen mode */

            }
    }
    else
        if (Addr >= 0x3F00)
        {
            return ppu.Memory[0x3F00 | PalAddr(Addr)];
        }
    return ppu.Memory[Addr];
}

void PPU_Wr(unsigned short Addr, unsigned char Value)
{
    if (Addr > (unsigned short) 0x3FFF)
    {
        Addr &= 0x3FFF;
    }
    if ((Addr < 0x3F00) && (Addr >= 0x2000))
    {
        if (Addr > 0x3000)
        {
            Addr -= 0x1000;
        }
        if (ppu.ScreenType == 0)
        {        /* 1 Screen Mode */
            ppu.Memory[RelAddr(Addr) + 0x2000] = Value;
        }
        else
            if (ppu.ScreenType == 1)
            {    /* Miroring mode */
                if (ppu.MirrorDir == 0)
                {    /* Vertical */
                    if (GetNT(Addr) & 0x1)
                    {    /* NT0-2 */
                        ppu.Memory[0x2400 + RelAddr(Addr)] = Value;
                    }
                    else
                    {
                        ppu.Memory[0x2000 + RelAddr(Addr)] = Value;
                    }
                }
                else
                {    /* Horizontal */
                    if (GetNT(Addr) & 0x2)
                    {    /* NT2-3 */
                        ppu.Memory[0x2000 + RelAddr(Addr)] = Value;
                    }
                    else
                    {
                        ppu.Memory[0x2400 + RelAddr(Addr)] = Value;
                    }
                }
            }
            else
            {    /* Four Screen mode */

            }
    }
    else
        if (Addr >= 0x3F00)
        {
            //console_printf(Console_Default, "%s palette: color %x new value : %d (0x%x)\n", (PalAddr(Addr) < 0x10) ? "Bgnd" : "Sprt", PalAddr(Addr), Value & 0x3F, Addr);
            ppu.Memory[ /* 0x3F00 | PalAddr(Addr) */ Addr] = Value & 0x3F;
            if (PalAddr(Addr) == 0x10)
                ppu.Memory[0x3F00] = Value & 0x3F;

        }
        else
        {
            ppu.Memory[Addr] = Value;
        }
}

unsigned short NbOfSprite[249];

void
 PPUDispSprite(unsigned char bg)
{
int x, y, x1, y1, px, py, i;
char Color;
PPUSprite sprite;

short SprtAddr;
//int VFlag, HFlag;

    for (i = 63; i >= 0; i--)
    {
        sprite = PPUGetSprite(i);
        if ((sprite.flags.BgPrio == bg))
        {        /* On doit afficher ce sprite */
            y = sprite.y;
            if (y < 248)
            {
                SprtAddr = ((sprite.tileid) << 4) + ppu.Sprt_Pattern_Table;
                x = sprite.x;
                for (y1 = 0; y1 < 8; y1++)
                {
                    py = y + (sprite.flags.VFlip == 0 ? y1 : (7 - y1));
                    if ((py > 0) && (py < 249) && ((++NbOfSprite[py]) > 7))
                    {
                        ppu.StatusRegister.s.SprtCount = 1;
                    }
                    for (x1 = 0; x1 < 8; x1++)
                    {
                        Color = GetTileColor(SprtAddr, x1, y1);
                        if (Color)
                        {
                            Color = (Color) | (sprite.flags.UpperColor << 2);
                            Color = ppu.Memory[0x3F10 + Color];
                            px = x + (sprite.flags.HFlip == 1 ? (7 - x1) : x1);
                            if ((i == 0) && (Buffer->line[py][px] != BgColor) && (ppu.HitSpriteAt == 255))
                            {
                                //Ligne utilis é pour le d é buguage
                                   // line(Buffer, 0, py, 256, py, 10);
                                ppu.HitSpriteAt = py + 1;
                            }
                            putpixel(Buffer, px, py, Color);
                            //Buffer->line[py][px] = Color;

                        }
                    }
                }
            }
        }
    }
}


void NewPPUDispSprite()
{
int x, y, x1, y1, px, py, i;
char Color;
PPUSprite sprite;
unsigned short bg;
short SprtAddr;


    for (i = 63; i >= 0; i--)
    {
        sprite = PPUGetSprite(i);
        bg = sprite.flags.BgPrio;

        y = sprite.y;
        if (y < 248)
        {
            SprtAddr = ((sprite.tileid) << 4) + ppu.Sprt_Pattern_Table;
            x = sprite.x;

            for (y1 = 0; y1 < 8; y1++)
            {
                py = y + (sprite.flags.VFlip == 0 ? y1 : ((8 - 1) - y1));
                if ((py > 0) && (py < 249) && ((++NbOfSprite[py]) > 7))
                {
                    ppu.StatusRegister.s.SprtCount = 1;
                }
                for (x1 = 0; x1 < 8; x1++)
                {
                    px = x + (sprite.flags.HFlip == 1 ? (7 - x1) : x1);
                    Color = GetTileColor(SprtAddr, x1, y1);
                    if (Color)
                    {
                        Color = (Color) | (sprite.flags.UpperColor << 2);
                        Color = ppu.Memory[0x3F10 + Color];
                        if ((i == 0) && (Buffer->line[py][px] != BgColor) && (ppu.HitSpriteAt == 255))
                        {
                            //Ligne utilisé pour le débuguage
                            //line(Buffer, 0, py+1, 256, py+1, 10);
                            ppu.HitSpriteAt = py+1;
                        }
                        if ((bg == 0) || ((bg == 1) && (Buffer->line[py][px] == BgColor)))
                            putpixel(Buffer, px, py, Color);
                    }
                }
            }
        }
    }
}


void NewPPUDispSprite_8x16()
{
int x, y, x1, y1, px, py, i, loop, tile;
char Color;
PPUSprite sprite;
unsigned short bg;
short SprtAddr;

unsigned short SprtTable;

    for (i = 63; i >= 0; i--)
    {
        sprite = PPUGetSprite(i);
        bg = sprite.flags.BgPrio;
        tile = sprite.tileid;
        y = sprite.y + 1;
        
        if (y < 248)
        {
            if ( (SprtTable = (tile & 0x1) * 0x1000) == 0x1000)
                tile -=1;
            if (sprite.flags.VFlip == 1)
             {
                y +=8;
             }
            for (loop = 0; loop < 2; loop++)
            {
                SprtAddr = ((tile) << 4) + SprtTable;
                x = sprite.x;

                for (y1 = 0; y1 < 8; y1++)
                {
                    py = y + (sprite.flags.VFlip == 0 ? y1 : ((8 - 1) - y1));
                    if ((py > 0) && (py < 249) && ((++NbOfSprite[py]) > 7))
                    {
                        ppu.StatusRegister.s.SprtCount = 1;
                    }
                    for (x1 = 0; x1 < 8; x1++)
                    {
                        px = x + (sprite.flags.HFlip == 1 ? (7 - x1) : x1);
                        Color = GetTileColor(SprtAddr, x1, y1);
                        if (Color)
                        {
                            Color = (Color) | (sprite.flags.UpperColor << 2);
                            Color = ppu.Memory[0x3F10 + Color];
                            if ((i == 0) && (Buffer->line[py][px] != BgColor) && (ppu.HitSpriteAt == 255))
                            {
                                //Ligne utilis é pour le débuguage
                                //line(Buffer, 0, py, 256, py, 10);
                                ppu.HitSpriteAt = py+1;
                            }
                            if ((bg == 0) || ((bg == 1) && (Buffer->line[py][px] == BgColor)))
                                putpixel(Buffer, px, py, Color);
                        }
                    }
                }
                tile += 1;
                if ( sprite.flags.VFlip == 1)
                    y -= 8;
                else
                    y += 8;
            }

        }
    }
}



int
 PPUVBlank()
{
int x, y, x1, y1, i;

unsigned long Reg2;
unsigned short ab_x, ab_y;
unsigned short Color;
unsigned short AttrByte;
unsigned short TileID;

unsigned char XScroll, YScroll;

        ppu.StatusRegister.s.VBlankOccur = 1;

        BgColor = ppu.Memory[0x3F00];//0xC0;
    clear_to_color(Buffer, BgColor);

/*    if (ppu.ControlRegister2.s.Colour != 0)
        console_printf(Console_Default, "ppu.ColorEmphasis : %d", ppu.ControlRegister2.s.Colour);*/


    for (i = 0; i < 249; i++)
        NbOfSprite[i] = 0;

    ppu.StatusRegister.s.SprtCount = 0;
    ppu.HitSpriteAt = 255;

/*
* A faires les choses qui faut faire durant un lancé de vblank,
* comme dessiner par ex..
*/

#define GetTilePos(addr,x,y) (addr+x+(y*32))

    if (ppu.DisplayAttributeTable)
    {
/* NT 2000 */
        for (x = 0; x < 0x40; x++)
        {
            AttrByte = /*ppu.Memory[0x23C0 + x];*/PPU_Rd(0x23C0 + x);
            x1 = x % 8;
            y1 = x / 8;
            
            
            Color = AttrByte & 0x3; // Pattern 1;
            if (ppu.DisplayNameTables == 0)
                Color = ppu.Memory[0x3F00 + (Color * 4) + 1];
            
            rectfill(Buffer,256+(x1*32),240+(y1*32),256+15+(x1*32),240+15+(y1*32),Color);
            
            Color = (AttrByte>>2) & 0x3; // Pattern 2;
            if (ppu.DisplayNameTables == 0)
                Color = ppu.Memory[0x3F00 + (Color * 4) + 1];
            
            rectfill(Buffer,16+256+(x1*32),240+(y1*32),16+256+15+(x1*32),240+15+(y1*32),Color);
            
            Color = (AttrByte>>4) & 0x3; // Pattern 3;
            if (ppu.DisplayNameTables == 0)
                Color = ppu.Memory[0x3F00 + (Color * 4) + 1];
            
            rectfill(Buffer,256+(x1*32),16+240+(y1*32),256+15+(x1*32),16+240+15+(y1*32),Color);
            
            Color = (AttrByte>>6) & 0x3; // Pattern 4;
            if (ppu.DisplayNameTables == 0)
                Color = ppu.Memory[0x3F00 + (Color * 4) + 1];
            
            rectfill(Buffer,16+256+(x1*32),16+240+(y1*32),16+256+15+(x1*32),16+240+15+(y1*32),Color);
        }


/* NT 2800 */
        for (x = 0; x < 0x40; x++)
         {
            AttrByte = PPU_Rd(0x2BC0 + x);
            x1 = x % 8;
            y1 = x / 8;
            
            
            Color = AttrByte & 0x3; // Pattern 1;
            if (ppu.DisplayNameTables == 0)
                Color = ppu.Memory[0x3F00 + (Color * 4) + 1];
            
            rectfill(Buffer,256+(x1*32),(y1*32),256+15+(x1*32),15+(y1*32),Color);
            
            Color = (AttrByte>>2) & 0x3; // Pattern 2;
            if (ppu.DisplayNameTables == 0)
                Color = ppu.Memory[0x3F00 + (Color * 4) + 1];
            
            rectfill(Buffer,16+256+(x1*32),(y1*32),16+256+15+(x1*32),15+(y1*32),Color);
            
            Color = (AttrByte>>4) & 0x3; // Pattern 3;
            if (ppu.DisplayNameTables == 0)
                Color = ppu.Memory[0x3F00 + (Color * 4) + 1];
            
            rectfill(Buffer,256+(x1*32),16+(y1*32),256+15+(x1*32),16+15+(y1*32),Color);
            
            Color = (AttrByte>>6) & 0x3; // Pattern 4;
            if (ppu.DisplayNameTables == 0)
                Color = ppu.Memory[0x3F00 + (Color * 4) + 1];
            
            rectfill(Buffer,16+256+(x1*32),16+(y1*32),16+256+15+(x1*32),16+15+(y1*32),Color);
         }
        
/* NT 2400 */
        for (x = 0; x < 0x40; x++)
         {
            AttrByte = PPU_Rd(0x27C0 + x);
            x1 = x % 8;
            y1 = x / 8;
            
            
            Color = AttrByte & 0x3; // Pattern 1;
            if (ppu.DisplayNameTables == 0)
                Color = ppu.Memory[0x3F00 + (Color * 4) + 1];
            
            rectfill(Buffer,256+256+(x1*32),240+(y1*32),256+256+15+(x1*32),240+15+(y1*32),Color);
            
            Color = (AttrByte>>2) & 0x3; // Pattern 2;
            if (ppu.DisplayNameTables == 0)
                Color = ppu.Memory[0x3F00 + (Color * 4) + 1];
            
            rectfill(Buffer,16+256+256+(x1*32),240+(y1*32),16+256+256+15+(x1*32),240+15+(y1*32),Color);
            
            Color = (AttrByte>>4) & 0x3; // Pattern 3;
            if (ppu.DisplayNameTables == 0)
                Color = ppu.Memory[0x3F00 + (Color * 4) + 1];
            
            rectfill(Buffer,256+256+(x1*32),240+16+(y1*32),256+256+15+(x1*32),240+16+15+(y1*32),Color);
            
            Color = (AttrByte>>6) & 0x3; // Pattern 4;
            if (ppu.DisplayNameTables == 0)
                Color = ppu.Memory[0x3F00 + (Color * 4) + 1];
            
            rectfill(Buffer,16+256+256+(x1*32),240+16+(y1*32),16+256+256+15+(x1*32),240+16+15+(y1*32),Color);
         }

/* NT 2C00 */
        for (x = 0; x < 0x40; x++)
        {
            AttrByte = PPU_Rd(0x2FC0 + x);//PPU_Rd(0x27C0 + x);
            x1 = x % 8;
            y1 = x / 8;
            
            
            Color = AttrByte & 0x3; // Pattern 1;
            if (ppu.DisplayNameTables == 0)
                Color = ppu.Memory[0x3F00 + (Color * 4) + 1];
            
            rectfill(Buffer,256+256+(x1*32),(y1*32),256+256+15+(x1*32),15+(y1*32),Color);
            
            Color = (AttrByte>>2) & 0x3; // Pattern 2;
            if (ppu.DisplayNameTables == 0)
                Color = ppu.Memory[0x3F00 + (Color * 4) + 1];
            
            rectfill(Buffer,16+256+256+(x1*32),(y1*32),16+256+256+15+(x1*32),15+(y1*32),Color);
            
            Color = (AttrByte>>4) & 0x3; // Pattern 3;
            if (ppu.DisplayNameTables == 0)
                Color = ppu.Memory[0x3F00 + (Color * 4) + 1];
            
            rectfill(Buffer,256+256+(x1*32),16+(y1*32),256+256+15+(x1*32),16+15+(y1*32),Color);
            
            Color = (AttrByte>>6) & 0x3; // Pattern 4;
            if (ppu.DisplayNameTables == 0)
                Color = ppu.Memory[0x3F00 + (Color * 4) + 1];
            
            rectfill(Buffer,16+256+256+(x1*32),16+(y1*32),16+256+256+15+(x1*32),16+15+(y1*32),Color);
            }

        if (ppu.DisplayNameTables == 0)
        {
            for (x = 0; x < 33; x++)
            {
                line(Buffer, 256 + x * 16, 0, 256 + x * 16, 240 + 240, 8);
                line(Buffer, 256, 0 + x * 16, 256 + 256 + 256, 0 + x * 16, 8);
            }

            for (x = 0; x < 17; x++)
            {
                line(Buffer, 256 + x * 32, 0, 256 + x * 32, 240 + 240, 6);
                line(Buffer, 256, 0 + x * 32, 256 + 256 + 256, 0 + x * 32, 6);
            }
        }
    }

    if (ppu.DisplayNameTables)
    {
/* NT 2000 */
        for (x = 0; x < 32; x++)
            for (y = 0; y < 30; y++)
            {
                TileID = (PPU_Rd/*ppu.Memory[*/(0x2000 + x + (y * 32))/*]*/ << 4) + ppu.Bg_Pattern_Table;

                for (x1 = 0; x1 < 8; x1++)
                    for (y1 = 0; y1 < 8; y1++)
                    {
                        Color = GetTileColor(TileID, x1, y1);
                        if (ppu.DisplayAttributeTable != 0)
                            Color |= (Buffer->line[(8 * y) + 240 + y1][(8 * x) + 256 + x1] & 0x3) << 2;

                        if ((Color != 0) || (ppu.DisplayAttributeTable != 0))
                        {
                            Color = ppu.Memory[0x3F00 + Color];
                            Buffer->line[(8 * y) + 240 + y1][(8 * x) + 256 + x1] = Color;
                        }
                    }
            }


/* NT 2800 */
        for (x = 0; x < 32; x++)
            for (y = 0; y < 30; y++)
            {
                TileID = (PPU_Rd(0x2800 + x + (y * 32)) << 4) + ppu.Bg_Pattern_Table;

                for (x1 = 0; x1 < 8; x1++)
                    for (y1 = 0; y1 < 8; y1++)
                    {
                        Color = GetTileColor(TileID, x1, y1);
                        if (Color != 0)
                        {
                            Color = ppu.Memory[0x3F00 + Color + 4];
                            Buffer->line[(8 * y) + y1][(8 * x) + 256 + x1] = Color;
                        }
                    }
            }

/* NT 2400 */
            for (x = 0; x < 32; x++)
                for (y = 0; y < 30; y++)
                    {
                    TileID = (/*PPU_Rd*/ppu.Memory[(0x2400 + x + (y * 32))] << 4) + ppu.Bg_Pattern_Table;
                    
                    for (x1 = 0; x1 < 8; x1++)
                        for (y1 = 0; y1 < 8; y1++)
                            {
                            Color = GetTileColor(TileID, x1, y1);
                            if (ppu.DisplayAttributeTable != 0)
                                Color |= (Buffer->line[(8 * y) + 240 + y1][(8 * x) + 256 + x1] & 0x3) << 2;
                            
                            if ((Color != 0) || (ppu.DisplayAttributeTable != 0))
                                {
                                Color = ppu.Memory[0x3F00 + Color];
                                Buffer->line[(8 * y) + 240+ y1][(8 * x) + 256 + 256 + x1] = Color;
                                }
                            }
                    }

/* NT 2C00 */
        for (x = 0; x < 32; x++)
            for (y = 0; y < 30; y++)
            {
                TileID = (PPU_Rd(0x2C00 + x + (y * 32)) << 4) + ppu.Bg_Pattern_Table;

                for (x1 = 0; x1 < 8; x1++)
                    for (y1 = 0; y1 < 8; y1++)
                    {
                        Color = GetTileColor(TileID, x1, y1);
                        if (Color != 0)
                        {
                            Color = ppu.Memory[0x3F00 + Color + 12];
                            Buffer->line[(8 * y) + y1][(8 * x) + 256 + 256 + x1] = Color;
                        }
                    }
            }
    }

    if (ppu.ControlRegister2.s.BgVisibility == 1)
    {
/* Display BG with scrolling informations */
/* J'ai la solution :D */
/*
frame start (line 0) (if background or sprites are enabled):
v=t
*/
        ppu.VRAMAddrReg2.W = ppu.TimedTmpPtr[0] | 0x2000;
        //console_printf(Console_Default, "Starting addresses : 0x%X\n",ppu.VRAMAddrReg2.W);
        
        XScroll = ppu.TimedHScroll[0];

        YScroll = ppu.TmpVScroll;

        for (y = 0; y < 240; y++)
        {
/*
scanline start (if background and sprites are enabled):
  8421 8421 8421 8421   8421 8421 8421 8421
v:0000 0100 0001 1111=t:0000 0100 0001 1111
  1111 1198 7654 3210         
  5432 10
*/
            
            ppu.VRAMAddrReg2.W = (ppu.VRAMAddrReg2.W & 0xFBE0)
               | ((ppu.TimedTmpPtr[y]) & 0x041F)
               | 0x2000;

            
            TileID = (PPU_Rd(ppu.VRAMAddrReg2.W) << 4)
               | ppu.Bg_Pattern_Table;

            XScroll = ppu.TimedHScroll[y];
            //YScroll = ppu.TimedVScroll[y];
            
            for (x = 0; x < 256; x++)
            {
/* Calculer la couleur du point */
/* Bits 1 et 0 */
                Color = GetTileColor(TileID, XScroll, YScroll);
/* Bits 3 et 2 */
/*
XScroll : 0,1,2,3,4
X = (ppu.VRAMAddrReg2.W & 0x1F)

Y : 5,6,7,8,9
Y = (ppu.VRAMAddrReg2.W & 0x3E0) >> 5
*/
/*ab_y = ((ppu.VRAMAddrReg2.W & 0x3E0) >> 5);
                ab_x = (ppu.VRAMAddrReg2.W & 0x1F);

                AttrByte = (((ab_y) >> 2) << 3) +
                   ((ab_x) >> 2);


                AttrByte = (PPU_Rd((ppu.VRAMAddrReg2.W & 0x2C00) + 0x3C0 + AttrByte) >>
                        ((((ab_y & 2) * 2) + (((ppu.VRAMAddrReg2.W & 0x2C00)) & 2)))) & 0x03;*/
                
                /*
                 
                 00DC BA98 7654 3210
                 
                 0000 BA54 3c-2 10b-
                 0000 0000 0001 1100  : 0x001C >> 2 
                 0000 0011 1000 0000  : 0x0380 >> 4
                 
                   10 --11 11-- ----
                 
                 0xC000
                 & 0x0C3F | 0x23C0
                 
                                b                  
                 val >> ( (Reg2 & 0x2) | ( (Reg2 & 0x0x40)>>4)
                 */
                
                Reg2 = ppu.VRAMAddrReg2.W;
                AttrByte = ((Reg2 & 0x0380) >> 4) | ((Reg2 & 0x001C) >> 2) | (Reg2 & 0x0C00);
                AttrByte &= 0x0C3F;
                AttrByte |= 0x23C0;
                AttrByte = PPU_Rd(AttrByte);
                AttrByte = AttrByte >> ( 0x0 | (Reg2 & 0x02) | ( (Reg2 & 0x40) >> 4) );
                AttrByte &= 0x3;
                
                if (Color)
                {
                    Color |= (AttrByte << 2);
                    Color = ppu.Memory[0x3F00 + Color];
                    Buffer->line[y][x] = Color + 0xC0;
                }
                
                XScroll++;
                XScroll &= 7;
                if (XScroll == 0)
                {    /* On incrémente le compteur de tile */
                    if ((ppu.VRAMAddrReg2.W & 0x1F) == 0x1F)
                    {    /* On met a 0 et change
                         * l'etat du bit 10 */
                        ppu.VRAMAddrReg2.W &= ~0x1F;

/* A voir si ya pas bcp mieux */
                        if ((ppu.VRAMAddrReg2.W & 0x400))
                            ppu.VRAMAddrReg2.W &= ~0x400;
                        else
                            ppu.VRAMAddrReg2.W |= 0x400;
                    }
                    else
                    {    /* on incremente juste */
                        ppu.VRAMAddrReg2.W = (ppu.VRAMAddrReg2.W & ~0x1F) | ((ppu.VRAMAddrReg2.W + 1) & 0x1F);
                    }

                    TileID = (PPU_Rd(ppu.VRAMAddrReg2.W) << 4)
                       | ppu.Bg_Pattern_Table;
                }
            }
/*
you can think of bits 5,6,7,8,9 as the "y scroll"(*8).  this functions
slightly different from the X.  it wraps to 0 and bit 11 is switched when
it's incremented from _29_ instead of 31.  there are some odd side effects
from this.. if you manually set the value above 29 (from either 2005 or
2006), the wrapping from 29 obviously won't happen, and attrib data will be
used as name table data.  the "y scroll" still wraps to 0 from 31, but
without switching bit 11.  this explains why writing 240+ to 'Y' in 2005
appeared as a negative scroll value.
*/
            YScroll++;
            YScroll &= 7;
            if (YScroll == 0)
            {    /* On incrémente le compteur de tile :| */
                if ((ppu.VRAMAddrReg2.W & 0x3E0) == 0x3A0)
                {    /* On met a 0 et change l'etat du bit
                     * 10 */
                    ppu.VRAMAddrReg2.W &= ~0x3F0;

/* A voir si ya pas bcp mieux */
                    if ((ppu.VRAMAddrReg2.W & 0x800))
                        ppu.VRAMAddrReg2.W &= ~0x800;
                    else
                        ppu.VRAMAddrReg2.W |= 0x800;
                }
                else
                {    /* on incremente juste */
                    ppu.VRAMAddrReg2.W = (ppu.VRAMAddrReg2.W & ~0x3F0) | ((((ppu.VRAMAddrReg2.W & 0x3F0) >> 5) + 1) << 5);
                }
            }
        }
        //while (!key[KEY_ENTER]);
    }
/*
* if (ppu.ControlRegister2.s.SpriteVisibility == 1)
* PPUDispSprite(0);
*/

    if (ppu.ControlRegister2.s.SpriteVisibility == 1)
    {
        if (ppu.ControlRegister1.s.SpriteSize)
        {
            NewPPUDispSprite_8x16();
        }
        else
        {
            NewPPUDispSprite();
        }
    }

/*    for(x=0;x<256;x++)
        for(y=0;y<240;y++)
        {
            if ((i = getpixel(Buffer,x,y)) >= 0xC0)
                putpixel(Buffer,x,y,ppu.Memory[0x3F00+i-0xC0]);
        }*/
    textprintf(Buffer, font, 5, 340, 4, "FPS : %d   IPS : %d", FPS, IPS);
    //textprintf(Buffer, font, 5, 3, 4, "FPS : %d (CPU@~%2.2fMhz : %d%%)", FPS, (float) (((float) IPS) / 1000000.0), (int) ((((float) IPS) / 1770000.0) * 100.0));

    if (ppu.DisplayPalette)
    {
        textout(Buffer, font, "Bg Palette", 0, 247, 5);
        textout(Buffer, font, "Sprt Palette", 90, 247, 5);

        rect(Buffer, 0, 255, 4 * 20 + 2, 255 + 4 * 20 + 2, 0);
        rect(Buffer, 90, 255, 90 + 4 * 20 + 2, 255 + 4 * 20 + 2, 0);
        for (i = 0; i < 16; i++)
        {
            rectfill(Buffer, 1 + (i % 4) * 20, 256 + (i / 4) * 20, 1 + (i % 4) * 20 + 20, 256 + (i / 4) * 20 + 20, ppu.Memory[0x3F00 + i]);
            rectfill(Buffer, 91 + (i % 4) * 20, 256 + (i / 4) * 20, 91 + (i % 4) * 20 + 20, 256 + (i / 4) * 20 + 20, ppu.Memory[0x3F10 + i]);
        }
    }
    if (ppu.DisplayVRAM)
    {

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

                    putpixel(Buffer, 10 + x1 + x, 347 + y1 + y, /*ppu.Memory[0x3F00 + */Color/*]*/);

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

                    putpixel(Buffer, 10 + 128 + x1 + x, 347 + y1 + y, /*ppu.Memory[*/0x3F10 + Color/*]*/);

                }
            x1 += 8;
            if (x1 >= 128)
            {
                x1 = 0;
                y1 += 8;
            }
        }
    }
    blit(Buffer, screen, 0, 0, 0, 0, 512 + 256, 480);

    //stretch_blit(Buffer, screen, 0, 0, 256, 240, 0, 0, 512, 480);

    if (ppu.ControlRegister1.s.VBlank_NMI == 1)
        return 1;

    return 0;
}

char
 ReadPPUReg(char RegID)
{
/* RegID is the nb of the reg 0-7 */
unsigned char ret;

    switch (RegID)
    {
    default:
        ret = (unsigned char) 0x00;    /* Can return every thing you
                         * want here :D */
        break;
    case 1:        /* Control Register 2 */
        ret = ppu.ControlRegister2.b;
        break;
    case 2:
        ret = ppu.StatusRegister.b;

        ppu.StatusRegister.s.VBlankOccur = 0;
        //ppu.StatusRegister.s.Sprite0Occur = 0;
        ppu.VRAMAddrMode = 0;
        break;
    case 5:
        ret = ppu.VRAMAddrReg2.B.l;
        break;
    case 6:
        ret = ppu.VRAMAddrReg2.B.h;
        break;
    case 7:        /* PPU in NES is really strange.. Bufferised
                 * send is nerd.. */
        if (ppu.VRAMAddrReg2.W < 0x3EFF)
        {
            ret = ppu.VRAMBuffer;
            ppu.VRAMBuffer = PPU_Rd(ppu.VRAMAddrReg2.W);
            ppu.VRAMAddrReg2.W += ppu.PPU_Inc;
            ppu.VRAMAddrReg2.W &= 0x3FFF;
        }
        else
         {
            ret = PPU_Rd(ppu.VRAMAddrReg2.W);
            //ppu.VRAMAddrReg2.W += ppu.PPU_Inc;
         }
        break;
    }
    return ret;
}

void
 WritePPUReg(char RegID, unsigned char val)
{
/* RegID is the nb of the reg 0-7 */

    switch (RegID)
    {
        default:/* For not writeable reg */
        break;
    case 0:        /* Control Register 1 */
        ppu.ControlRegister1.b = val;
        ppu.Bg_Pattern_Table = (ppu.ControlRegister1.s.BgPattern == 1) ? 0x1000 : 0x0000;
        ppu.Sprt_Pattern_Table = (ppu.ControlRegister1.s.SptPattern == 1) ? 0x1000 : 0x0000;
        ppu.PPU_Inc = (ppu.ControlRegister1.s.AddrIncrmt == 1) ? 32 : 1;

        ppu.Name_Table_Addresse = 0x2000;
        switch (ppu.ControlRegister1.s.NameTblAddr)
        {
        case 3:
            ppu.Name_Table_Addresse += 0x400;
        case 2:
            ppu.Name_Table_Addresse += 0x400;
        case 1:
            ppu.Name_Table_Addresse += 0x400;
        case 0:
            break;
        }
            ppu.TimedNT[ScanLine] = ppu.ControlRegister1.s.NameTblAddr;
/*
2000 write:
  1111 0011 1111 1111 ( F3FF )
t:0000 1100 0000 0000 = d:0000 0011
*/
        ppu.TmpVRamPtr = ( (ppu.TmpVRamPtr & 0xF3FF)
                       | ( ((ppu.ControlRegister1.s.NameTblAddr) & 0x03) << 10 )
                         );

        break;
    case 1:        /* Control Register 2 */
        //console_printf(Console_Default, "PPU: new CR2 ; 0x%x\n", val);
        ppu.ControlRegister2.b = val;
        break;
    case 3:        /* SPR-RAM Addresse Register */
        ppu.SPR_RAMAddr = val;
        break;
    case 4:        /* SPR-RAM Input Register */
        ppu.SPRRAM[ppu.SPR_RAMAddr] = val;
        break;
    case 5:        /* VRAM Address register 1 */
        if (ppu.VRAMAddrMode == 0)
        {
/*
2005 first write:
t:0000 0000 0001 1111=d:1111 1000
x=d:00000111
*/
            ppu.VRAMAddrMode = 1;

            ppu.TmpVRamPtr = ((ppu.TmpVRamPtr & 0xFFE0) | ((val & 0xF8) >> 3));
            ppu.HScroll = val & 0x7;

            //console_printf(Console_Default, "%d -> 2005 w1: 0x%04X (val: 0x%02X)\n", ScanLine, ppu.TmpVRamPtr, val);
        }
        else
        {
/*
2005 second write:
t:0000 0011 1110 0000=d:1111 1000
t:0111 0000 0000 0000=d:0000 0111
*/
            ppu.VRAMAddrMode = 0;
            ppu.TmpVRamPtr = ((ppu.TmpVRamPtr & 0xFC1F) | ((val & 0xF8) << 2));
            ppu.TmpVRamPtr = ((ppu.TmpVRamPtr & 0x8FFF) | ((val & 0x07) << 12));

            ppu.TmpVScroll = ((ppu.TmpVRamPtr & 0x700) >> 12) & 0x7;
            if (ppu.TmpVScroll != 0)
                console_printf(Console_Default, "2002: TmpVScroll == %d \n", ppu.TmpVScroll);
            
            //console_printf(Console_Default, "%d -> 2005 w2: 0x%04X (val: 0x%02X)\n", ScanLine, ppu.TmpVRamPtr, val);

        }
        break;
    case 6:        /* VRAM Address register 2 */
        if (ppu.VRAMAddrMode == 0)
        {
            ppu.VRAMAddrMode = 1;
/*
2006 first write:
t:0011 1111 0000 0000 = d:0011 1111
t:1100 0000 0000 0000=0
*/
            ppu.TmpVRamPtr = ((ppu.TmpVRamPtr & 0xC0FF) | ((val&0x3F) << 8)) & 0x3FFF;

            //console_printf(Console_Default, "%d -> 2006 w1: 0x%04X (val: 0x%02X)\n", ScanLine, ppu.TmpVRamPtr, val);
        }
        else
        {
            ppu.VRAMAddrMode = 0;
/*
2006 second write:
t:0000000011111111=d:11111111
v=t
*/
            ppu.TmpVRamPtr = ((ppu.TmpVRamPtr & 0xFF00) | (val & 0x00FF));
            ppu.VRAMAddrReg2.W = ppu.TmpVRamPtr;

            //console_printf(Console_Default, "%d -> 2006 w2: 0x%04X (val: 0x%02X)\n", ScanLine, ppu.TmpVRamPtr, val);

        }
        break;
    case 7:        /* VRAM I/O */
        PPU_Wr(ppu.VRAMAddrReg2.W, val);
        ppu.VRAMAddrReg2.W += ppu.PPU_Inc;
        break;
    }
}
