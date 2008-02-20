/*
 *  PPU emulation - The TI-NESulator Project
 *  ppu.c
 *  
 *  Define and emulate the PPU (Picture Processing Unit) of the real NES
 * 
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2007 986Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-05-31 18:02:16 +0200 (jeu, 31 mai 2007) $
 *  $Author: mtrapier $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/ppu.c $
 *  $Revision: 58 $
 *
 */

#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>

#define __TINES_PPU_INTERNAL__
#include <ppu/ppu.h>
#include <ppu/ppu.memory.h>
#include <ppu/ppu.debug.h>

#include <memory/manager.h>

#define __TINES_PLUGINS__
#include <plugins/manager.h>

#if ISPAL && !ISNTSC
//#define VBLANK_TIME 70
extern int VBLANK_TIME;
#elif !ISPAL && ISNTSC
//#define VBLANK_TIME 20
extern int VBLANK_TIME;
#else
#error Cannot use ISPAL with ISNTSC together !
#endif

#ifdef NO_N_KEY
#define IF_N_KEY if (!key[KEY_N])
#else
#define IF_N_KEY if (key[KEY_N])
#endif

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
        
    printf("ppu_mem_nameTables   :%p\n"
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

    
    /* Dump PPU memory state */
    //ppu_memoryDumpState(stdout);

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

//    plugin_install_keypressHandler('i', ppu_debugSprites);
//    plugin_install_keypressHandler('I', ppu_debugSprites);
    
//    plugin_install_keypressHandler('u', ppu_debugColor);
//    plugin_install_keypressHandler('U', ppu_debugColor);
      
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
        sprite_y    = ppu_mem_spritesTable[(i*4) + 0];
        sprite_idx  = ppu_mem_spritesTable[(i*4) + 1];
        sprite_attr = ppu_mem_spritesTable[(i*4) + 2] | ((i==0)?0x04:0); /* Add a flag for the sprite #0 */
        sprite_x    = ppu_mem_spritesTable[(i*4) + 3];
      
        /* For each line covered by the sprite */
        for (line = 0; line < ppu_spriteSize; line ++)
        {       
            curline = line + sprite_y;
            
            if ((curline < 0) || (curline > 240))
                continue; /* Don't go beyond, this sprite go beyond the borders */
                
            if (PPU_NbSpriteByScanLine[curline] < 7)
                PPU_NbSpriteByScanLine[curline] ++;
            else
            {
                PPU_NbSpriteByScanLineOverFlow[curline] = 1;
                //printf("sprite of: %d - %d\n", curline, PPU_NbSpriteByScanLine[curline]);
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
                    //printf("new sprite [%02X:%02X:%02X:%02X] at sl:%d : 0x%08X ", 
                    //sprite_attr, sprite_idx, curline - sprite_x, sprite_y,
                    //curline, PPU_SpriteByScanLine[curline][j]);
                    
                    PPU_SCANLINESPRITE_SET_TILIDX(PPU_SpriteByScanLine[curline][j], sprite_idx);
                    //printf("- 0x%08X ", PPU_SpriteByScanLine[curline][j]);
                    
                    PPU_SCANLINESPRITE_SET_RELY  (PPU_SpriteByScanLine[curline][j], curline - sprite_y);
                    //printf("- 0x%08X ", PPU_SpriteByScanLine[curline][j]);
                    
                    PPU_SCANLINESPRITE_SET_X     (PPU_SpriteByScanLine[curline][j], sprite_x);                    
                    //printf("- 0x%08X\n", PPU_SpriteByScanLine[curline][j]);
                    
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
        //printf("Set mirror to Hor\n");
        ppu_setPagePtr1k(0x20, ppu_mem_nameTables + 0x000);
        ppu_setPagePtr1k(0x24, ppu_mem_nameTables + 0x000);
        ppu_setPagePtr1k(0x28, ppu_mem_nameTables + 0x400);
        ppu_setPagePtr1k(0x2C, ppu_mem_nameTables + 0x400);
        break;
    case PPU_MIRROR_VERTICAL: /* Vertical */
        //printf("Set mirror to Ver\n");
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
        //printf("Set screen to 0x000\n");
        ppu_setPagePtr1k(0x20, ppu_mem_nameTables + 0x000);
        ppu_setPagePtr1k(0x24, ppu_mem_nameTables + 0x000);
        ppu_setPagePtr1k(0x28, ppu_mem_nameTables + 0x000);
        ppu_setPagePtr1k(0x2C, ppu_mem_nameTables + 0x000);
        break;
        
    case PPU_SCREEN_400: /* 0x2400 */
        //printf("Set screen to 0x400\n");
        ppu_setPagePtr1k(0x20, ppu_mem_nameTables + 0x400);
        ppu_setPagePtr1k(0x24, ppu_mem_nameTables + 0x400);
        ppu_setPagePtr1k(0x28, ppu_mem_nameTables + 0x400);
        ppu_setPagePtr1k(0x2C, ppu_mem_nameTables + 0x400);
        break;
        
    case PPU_SCREEN_800: /* 0x2800 */
        //printf("Set screen to 0x800\n");
        ppu_setPagePtr1k(0x20, ppu_mem_nameTables + 0x800);
        ppu_setPagePtr1k(0x24, ppu_mem_nameTables + 0x800);
        ppu_setPagePtr1k(0x28, ppu_mem_nameTables + 0x800);
        ppu_setPagePtr1k(0x2C, ppu_mem_nameTables + 0x800);
        break;
        
    case PPU_SCREEN_C00: /* 0x2C00 */
        //printf("Set screen to 0xC00\n");
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
        //printf("Set Single Screen\n");
        ppu_setSingleScreen(~ppu_singleScreenMode);
        break;

    default:
        mode = PPU_SCMODE_NORMAL;
        ppu_screenMode = mode;       

    case PPU_SCMODE_NORMAL: /* Normal screen (2 NT with mirroring) */
        //printf("Set Normal Screen\n");
        ppu_setMirroring(~ppu_mirrorMode);
        break;

    case PPU_SCMODE_FOURSC: /* Four   screen (4 NT withou mirroring) */
        //printf("Set Four   Screen\n");
        ppu_setPagePtr1k(0x20, ppu_mem_nameTables + 0x000);
        ppu_setPagePtr1k(0x24, ppu_mem_nameTables + 0x400);
        ppu_setPagePtr1k(0x28, ppu_mem_nameTables + 0x800);
        ppu_setPagePtr1k(0x2C, ppu_mem_nameTables + 0xC00);
        break;
    }
}

void ppu_setSprite(unsigned short i, PPU_Sprite *sprt)
{
 
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
    
    IF_N_KEY printf("Counter update to %04X\n",PPU_Reg_Counter);
}

int ppu_hblank(int scanline)
{  
    int i, j;
    byte pixelColor = 0x42;
    byte Color = 0x42;
    unsigned short addr;
    byte value;
    unsigned short tmp_HHT = 0;
    unsigned short tmp_VVTFV = 0;  
    
    /* If no plan activated, we have nothing to do ! */
    
    if (scanline == 0)
    {
      ppu_bgColor =  ppu_readMemory(0x3F,00);
      clear_to_color(VideoBuffer, ppu_bgColor);
      
      if ((ppu_spriteVisibility != 0) || (ppu_backgroundVisibility != 0))
          ppu_updateCounters();
    }

    if (scanline < 240)
    {
            
        /* For each PPU pixel of this scanline */        
        for (i = 0; i < 256; i ++)
        {    

            /* determine which from sprite bg, bg and sprite fg is in the front */
            pixelColor = ppu_readMemory(0x3F,00);
            
            /* is there sprite(s) on this line ? */            
                
            /*  */
            /* Didn't display sprite for now, juste the BG */
            
            /* Read NameTable */            
            if (ppu_backgroundVisibility == 1)
            {
/*
xxxx AABB BxxC CCxx
xxxx AA11 11BB BCCC
*/
/*
s:32 i:235 cnt:089D addr:0BCF cba:0 addr:0000

0000 BBBB CCCC FFFF
0000 1100 1101 1111

     AABB B  C CC
     
     BA98 7654 3210
     
xxxx 1111 1100 1111     
     FFFF CCCC FFFF

*/

                addr  = (PPU_Reg_Counter & 0x0C00);
                addr  = addr | 0x03C0;
                addr |= (PPU_Reg_Counter >> 4 ) & 0x0038;
                addr |= (PPU_Reg_Counter >> 2 ) & 0x0007;            
            
                PPU_Reg_AR = ppu_readMemory(0x20 | ((addr>>8) & 0x0F), addr& 0xFF);

                
                PPU_Reg_AR = PPU_Reg_AR >> (((PPU_Reg_Counter >> 4 ) & 0x04)|((PPU_Reg_Counter ) & 0x02));
                PPU_Reg_AR = (PPU_Reg_AR<<2) & 0x0C;
                    
                PPU_Reg_PAR = ppu_readMemory(0x20 | ((PPU_Reg_Counter>>8) & 0x0F), PPU_Reg_Counter& 0xFF);
                /* C BA98 7654 3210 */
                /* 1 8421 8421 8421 */
            
                addr  =  PPU_Reg_S;
                addr |= ((PPU_Reg_PAR & 0xFF) << 4);
                addr |= ((PPU_Reg_Counter >> 12)  & 0x07);
                value = ppu_readMemory((addr >> 8) , addr        );
                Color  = (value & (1 << (7-(i + PPU_Reg_FH) % 8)))?0x01:0;
            
                value = ppu_readMemory((addr >> 8) , addr | 0x08 );
                Color |= (value & (1 << (7-(i + PPU_Reg_FH) % 8)))?0x02:0;
            
                if (Color > 0x00)
                {
                
                    Color |= PPU_Reg_AR;
           	        Color &= 0x0F;

                    pixelColor = ppu_readMemory(0x3F, Color);

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
            /* draw the pixel */            
            _putpixel(VideoBuffer, i, scanline, pixelColor);                      
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
        if (ppu_backgroundVisibility == 1)
        {

        tmp_VVTFV = ((PPU_Reg_Counter >> 3 ) & 0x0100) | /* V */
                    ((PPU_Reg_Counter >> 2 ) & 0x00F8) | /* VT */
                    ((PPU_Reg_Counter >> 12) & 0x0007);  /* FV */
        //printf("counter:%04X vvtfv:%04X ", PPU_Reg_Counter, tmp_VVTFV);
        
        tmp_VVTFV++;
        //printf("__ vvtfv:0x%04X == 0x%04X ? ", tmp_VVTFV, 30<<3);
        if ((tmp_VVTFV&0x0F8) == 0xF0)
        {
	        tmp_VVTFV &= ~0x0F8;
	        tmp_VVTFV ^= 0x100;
            //printf("YES _");
        }
            
        //printf("vvtfv:%04X ", tmp_VVTFV);
        PPU_Reg_Counter = ( PPU_Reg_Counter & 0x041F)   |        
                          ((tmp_VVTFV & 0x0100 ) << 3 ) | /* V */
                          ((tmp_VVTFV & 0x00F8 ) << 2 ) | /* VT */
                          ((tmp_VVTFV & 0x0007 ) << 12);  /* FV */
        
        //printf("counter:%04X ", PPU_Reg_Counter);
        /* Update H & HT */
        PPU_Reg_Counter = (PPU_Reg_Counter & ~0x041F) | 
                          (PPU_Reg_H << 10)           |
                           PPU_Reg_HT;
        }
        
        if (PPU_NbSpriteByScanLine[scanline] != 0)
            {
                for (j = 0; j < PPU_NbSpriteByScanLine[scanline]; j++)
                {
                    static byte i = 0;
                    pixelColor = (i = (i+1)%4) | ((PPU_SCANLINESPRITE_GET_ATTRS(PPU_SpriteByScanLine[scanline][j]) << 2) & 0x0C);
                    
                    pixelColor = ppu_readMemory(0x3F, 0x10 + pixelColor);
                    line(VideoBuffer,
                          PPU_SCANLINESPRITE_GET_X(PPU_SpriteByScanLine[scanline][j]),
                          scanline,
                          PPU_SCANLINESPRITE_GET_X(PPU_SpriteByScanLine[scanline][j]) + 8,
                          scanline, pixelColor
                          );
                          
                    if (PPU_SCANLINESPRITE_GET_ATTRS(PPU_SpriteByScanLine[scanline][j]) & 0x04)
                    {
                        //printf("Hit!\n");
                        ppu_spriteZeroHit = 1;
                    }
                          
                }
            }
        ppu_scanlineSpriteOverflow = 0;
        if (PPU_NbSpriteByScanLineOverFlow[scanline] == 1)
            ppu_scanlineSpriteOverflow = 1;
        
    }
    
   /* if (scanline == 100)
        ppu_spriteZeroHit = 1;*/
    
    if (scanline == 243)
    {
        ppu_inVBlankTime = 1;
        IF_N_KEY printf("============= enter vblank =================\n");
        return ppu_execNMIonVBlank;
    }  

    if (key[KEY_B])
    {
        blit(VideoBuffer, Buffer, 0, 0, 0, 0, 256, 240);
        blit(Buffer, screen, 0, 0, 0, 0, 512 + 256, 480);
    }


    //if (scanline >= (241 + VBLANK_TIME))
    if (scanline >= 262)
    {   
        //textprintf(Buffer, font, 5, 340, 4, "(SL:%d) FPS : %d   IPS : %d", scanline, FPS, IPS);
        textprintf(screen, font, 260, 3, 4, "FPS : %d (CPU@~%2.2fMhz : %d%%)", FPS, (float) (((float) IPS) / 1000000.0), (int) ((((float) IPS) / 1770000.0) * 100.0));  
        //printf("(SL:%d) FPS : %d   IPS : %d\n", scanline, FPS, IPS);
        
        //ppu_dumpPalette(0, 241);
        //ppu_dumpPattern(280, 150);
        //ppu_dumpNameTable(256,0);
        //ppu_dumpAttributeTable(257, 0);
                    
        //blit(VideoBuffer, Buffer, 0, 0, 0, 0, 256, 240);
        blit(VideoBuffer, screen, 0, 0, 0, 0, 256, 240);
        //blit(Buffer, screen, 0, 0, 0, 0, 512 + 256, 480);
        
        IF_N_KEY printf("_____________ leave vblank _________________\n");
        ppu_inVBlankTime = 0;
        ppu_spriteZeroHit = 0;
        //ppu_updateCounters();       
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
        printf("%s: try to read 0x20%02X\n", __func__, id);
        break;
    case 0x02: /* PPU Status Register */
    
        /* Reset VRam 2005/2006 flipflop */
        ppu_VramAccessFlipFlop = 0;
        garbage = 0;
        
        garbage |= (ppu_inVBlankTime!=0)          ?PPU_FLAG_SR_VBLANK:0;
        garbage |= (ppu_spriteZeroHit!=0)         ?PPU_FLAG_SR_SPRT0:0;
        garbage |= (ppu_scanlineSpriteOverflow!=0)?PPU_FLAG_SR_8SPRT:0;
        /*garbage ^= PPU_FLAG_SR_RDWRALLOW;*/

        IF_N_KEY printf("%s() = %02X\n", __func__, garbage);

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
    //printf("ppuread %02X return: %02X\n", id, garbage);
    return garbage;    
}


void ppu_writeReg(byte id, byte val)
{
    id &= 0x07;
    //printf("ppuwrte %02X val: %02X\n", id, val);
    PPU_RegValues[id] = val;
    switch(id)
    {
    default:
        printf("%s: try to write 0x%02X @ 0x20%02X\n", __func__, val, id);
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
         IF_N_KEY
         printf("%s(%02X, %02X); /* 2000: "
	     "NMI:%c SPRTSIZE:%02d BGTA:%04X[0x%04X] SPTA:%04X INC:%02d NTA:%04X */\n"
	     , __func__, id, val,
	     (val & 0x80)?'E':'D',
	     (val & 0x20)?16:8,
	     (val & 0x10)?0x1000:0x0000, PPU_Reg_S,
	     (val & 0x08)?0x1000:0x0000,
	     (val & 0x04)?32:1,
	     (val & 0x03)<<10|0x2000
	     );

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

          IF_N_KEY
          printf("%s(%02X, %02X); /* 2001 : "
      	          "SprtV:%c BckgV:%c SprtC:%c BckgC:%c DispT:%c"
                  " */\n", __func__, id, val,
                  ppu_spriteVisibility?'y':'n',
                  ppu_backgroundVisibility?'y':'n',
                  ppu_spriteClipping?'y':'n',
                  ppu_backgroundClipping?'y':'n',
                  ppu_displayType?'m':'c'
                  );    
        
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
            IF_N_KEY
                printf("2005/1[%04X]: fv:%01X v:%01X h:%01X vt:%01X ht:%01X fh:%01X\n",val,PPU_Reg_FV,PPU_Reg_V,PPU_Reg_H,PPU_Reg_VT,PPU_Reg_HT,PPU_Reg_FH);
        }
        else
        {
            ppu_VramAccessFlipFlop = 0;
            
            PPU_Reg_FV =  val & 0x07;
            PPU_Reg_VT = (val & 0xF8) >> 3;
            IF_N_KEY
                printf("2005/2[%04X]: fv:%01X v:%01X h:%01X vt:%01X ht:%01X fh:%01X\n",val,PPU_Reg_FV,PPU_Reg_V,PPU_Reg_H,PPU_Reg_VT,PPU_Reg_HT,PPU_Reg_FH);
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
            IF_N_KEY
                printf("2006/1[%04X]: fv:%01X v:%01X h:%01X vt:%01X ht:%01X fh:%01X\n",val,PPU_Reg_FV,PPU_Reg_V,PPU_Reg_H,PPU_Reg_VT,PPU_Reg_HT,PPU_Reg_FH);
        }
        else
        {
            ppu_VramAccessFlipFlop = 0;
            PPU_Reg_VT = (PPU_Reg_VT & 0x18) | ((val >> 5) & 0x07);
            PPU_Reg_HT =  val & 0x1F;
            
            IF_N_KEY
                printf("2006/2[%04X]: fv:%01X v:%01X h:%01X vt:%01X ht:%01X fh:%01X\n",val,PPU_Reg_FV,PPU_Reg_V,PPU_Reg_H,PPU_Reg_VT,PPU_Reg_HT,PPU_Reg_FH);
                
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

        

        //if ( (PPU_Reg_Counter&0xFF00) == 0x3F00)
        //{
       //   printf("fv:%01X v:%01X h:%01X vt:%01X ht:%01X fh:%01X\n",PPU_Reg_FV,PPU_Reg_V,PPU_Reg_H,PPU_Reg_VT,PPU_Reg_HT,PPU_Reg_FH);
       //   printf("will write ppu: counter:%04X pa:%02X%02X v:%02X\n",
	   //      PPU_Reg_Counter, (PPU_Reg_Counter>>8) & 0x3F, PPU_Reg_Counter & 0xFF, val);
       // }
        
        ppu_writeMemory((PPU_Reg_Counter>>8) & 0x3F, PPU_Reg_Counter & 0xFF, val);       

        IF_N_KEY
        {
            ppu_dumpPalette(0, 241);
            ppu_dumpPattern(280, 150);
            ppu_dumpNameTable(256,0);
            blit(Buffer, screen, 0, 0, 0, 0, 512 + 256, 480);
        }
        
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
    //memcpy(ppu_mem_spritesTable, ptr, 0xFF);
    ppu_updateSpriteScanlineTable();
}
