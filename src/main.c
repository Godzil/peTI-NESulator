/*
 *  Main application source file - The TI-NESulator Project
 *  main.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>

#ifndef WIN32

//#include <Allegro/allegro.h>
#include <allegro.h>

#else

#define USE_CONSOLE
#include <allegro.h>

#endif

#if ISPAL && !ISNTSC
//#define VBLANK_TIME 70
//#define HBLANK_TIME 261
int VBLANK_TIME     = 70;
int HBLANK_TIME     = 140;
double APU_BASEFREQ = 1.7734474;
#elif !ISPAL && ISNTSC
int VBLANK_TIME     = 20;
int HBLANK_TIME     = 115; //119;
double APU_BASEFREQ = 1.7897725;
//#define VBLANK_TIME 20
//#define HBLANK_TIME 260
#else
#error Cannot use ISPAL with ISNTSC together !
#endif

/* Should find something better for sharing the copu interface... */

#include <corecpu/M6502.h>
#include <ppu/ppu.h>
#include <NESCarts.h>
#include <paddle.h>

#include <mappers/manager.h>

#include <memory/manager.h>

#include <plugins/manager.h>

#ifdef USE_SOUND
#include <Sound.h>
#endif

#include <palette.h>

#define V_MAJOR 0
#define V_MINOR 30

#define VS_ID              "$Id$"
#define VS_REVISION        "$Revision$"
#define VS_LASTCHANGEDDATE "$LastChangedDate$"
#define VS_HEADURL         "$HeadURL$"
#define VS_AUTHOR          "$Author$"

#define MAXLASTOP 42

word latestop[MAXLASTOP];

M6502 MainCPU;

BITMAP *Buffer;

byte *FDSRom;
byte *FDSRam;


/* Command line options */
byte START_DEBUG = 0;
byte START_WITH_FDS = 0;
char *CART_FILENAME = NULL;
char *PALETTE_FILENAME = NULL;

#define fatal(s) { printf("%s",s); exit(-1); }

Paddle P1, P2;

unsigned short ScanLine;

volatile int frame = 0;
volatile extern int icount;

char MapperWantIRQ = 0;

char WantClosing = 0;

struct timeval timeStart;
struct timeval timeEnd;

volatile unsigned long FPS, IPS;

PALETTE pal;

short IRQScanHit = -1;

/* palette */
unsigned long ColorPalette[ 8 * 63 ];

#define SET_RGB(r,g,b) ((((r<<8)|g)<<8)|b)|0xFF000000

void CloseHook(void)
{
   WantClosing = 1;
}

void ips_fps_counter(void)
{
   FPS = frame;
   IPS = icount;
   frame = 0;
   icount = 0;
}

END_OF_FUNCTION(ips_fps_counter);

void SaveSaveRam(char *name)
{
   FILE *fp;
   int i;
   char fname[512];
   //byte car;
   strcpy(fname, name);
   strcat(fname, ".svt");
   if ((fp = fopen(fname, "wb")))
   {
	 printf("Saving savestate '%s'\n", fname);
	 for( i = 0x60; i < 0x80; i++)
	 {
	    fwrite(get_page_ptr(i), 1, 0x100, fp);
	 }
	 
	 fclose(fp);
   }
}

void LoadSaveRam(char *name)
{
   FILE *fp;
   int i;
   char fname[512];
   
   strcpy(fname, name);
   strcat(fname, ".svt");
   if ((fp = fopen(fname, "rb")))
   {
	 printf("Loading savestate '%s'\n", fname);
	 for( i = 0x60; i < 0x80; i++)
	 {
	    fread(get_page_ptr(i), 1, 0x0100, fp);
	 }    
	 fclose(fp);
	 
   }
}


void LoadPalette(char *filename, PALETTE pal)
{
   FILE *fp;
   
   unsigned char r, v, b, i;
   printf("%s: try to load pallette file '%s'", __func__, filename);
   if ((fp = fopen(filename, "rb")) != NULL)
   {
	 
	 for (i = 0; i < 64; i++)
	 {
	    
	    fread(&r, 1, 1, fp);
	    fread(&v, 1, 1, fp);
	    fread(&b, 1, 1, fp);
	    
	    /*            r = (r * 64) / 255;
		v = (v * 64) / 255;
		b = (b * 64) / 255;*/
	    
	    
#ifdef USE_24BITS            
	    ColorPalette[i + (0 * 63)] = SET_RGB(r,v,b);
	    
	    /* Red emphase */
	    ColorPalette[i + (1 * 63)] = SET_RGB(r + 10, v - 05, b - 05);
	    
	    /* Green emphase */
	    ColorPalette[i + (2 * 63)] = SET_RGB(r - 05, v + 10, b - 05);
	    
	    /* Red + green emphase */
	    ColorPalette[i + (3 * 63)] = SET_RGB(r + 05, v + 05, b - 10);
	    
	    /* Blue emphase */
	    ColorPalette[i + (4 * 63)] = SET_RGB(r - 05, v - 05, b + 10);
	    
	    /* Red + blue emphase */
	    ColorPalette[i + (5 * 63)] = SET_RGB(r + 05, v - 10, b + 05);
	    
	    /* Blue + green emphase */
	    ColorPalette[i + (6 * 63)] = SET_RGB(r - 10, v + 05, b + 05);
	    
	    /* Red + Green + Blue emphase */
	    ColorPalette[i + (7 * 63)] = SET_RGB(r + 00, v + 00, b + 00);*/
#else /* Else Use 8Bits */
	    pal[i].r = r;
	    pal[i].g = v;
	    pal[i].b = b;
	    
	    pal[i + 64].r = r;
	    pal[i + 64].g = v;
	    pal[i + 64].b = b;
	    
	    pal[i + 128].r = r;
	    pal[i + 128].g = v;
	    pal[i + 128].b = b;
	    
	    pal[i + 192].r = r;
	    pal[i + 192].g = v;
	    pal[i + 192].b = b;
#endif
	 }
	 fclose(fp);
	 printf(" [ OK ]\n");
   }
   else
   {
	 printf("Error loading palette '%s'!\n", filename);
	 exit(-1);
   }
}

int DAsm(char *S, word A);

int oppos = 0;

void pushop(word op)
{
   latestop[oppos] = op;
   //  printf("%d\n", oppos);  
   oppos = (oppos+1)%42;
}

void showlastop(FILE *fp)
{
#ifdef DEBUG
   int i,j;
   char S[256];
   i = oppos;
   do
   {
	 j=(DAsm(S,latestop[i])-1);
	 fprintf(fp, "0x%04X : %s\n", MainCPU.PC.W,S);
	 i = (i+1)%42;
   }
   while(i != oppos);
#endif
}

NesCart *Cart;

void *signalhandler(int sig)
{
   static int state=0;
   M6502 *R = &MainCPU;
   byte F;
   int J, I;
   static char FA[8] = "NVRBDIZC";    
   char S[128];
   char name[512];
   static FILE *fp = NULL;
   sprintf(name, "crashdump-%d.txt", (int)time(NULL));
   if (state != 0)
   {
	 fprintf(stderr, "\n\n\nCrashed within signal!\nEmergency exit\n");
	 exit(42);
   }
   state = 1;
   
   if (fp == NULL)
	 fp = fopen(name, "wt");
   
   state = 2;
   
   if (fp) fprintf(stderr,
			    "\n\n\n\n\n"
			    "#sick# TI-NESulator %d.%d #sick#\n"
			    "see %s for more information",
			    V_MAJOR,
			    V_MINOR,
			    name);
   
   if (!fp) fp = stderr;
   
   fprintf(fp,"\n\n\n\n\n"
		 "#sick# TI-NESulator %d.%d #sick# signal: ",
		 V_MAJOR,
		 V_MINOR);
   switch(sig)
   {
	 default:
	 case SIGABRT: fprintf(fp,"Abnormal termination"); break;
	 case SIGILL:  fprintf(fp,"Illegal instruction"); break;
	 case SIGINT:  fprintf(fp,"CTRL+C signal"); break;
	 case SIGSEGV: fprintf(fp,"Segmentation fault"); break;
	 case SIGTERM: fprintf(fp,"Termination request"); break;
   }
   fprintf(fp,"\nAn error occured during the excution.\n Crash report information :\n");
#ifdef DEBUG
   DAsm(S, R->PC.W);
#endif
   fprintf(fp, "CPU: A:%02X  P:%02X  X:%02X  Y:%02X  S:%04X  PC:%04X  Flags:[", 
		 R->A, R->P, R->X, R->Y, R->S + 0x0100, R->PC.W);
   for (J = 0, F = R->P; J < 8; J++, F <<= 1)
	 fprintf(fp, "%c", F & 0x80 ? FA[J] : '.');
   fprintf(fp, "]\nCPU: ");
   fprintf(fp, "AT PC: [%02X - %s]   AT SP: [%02X %02X %02X]\nLast execution :\n", 
           Rd6502(R->PC.W), S, 
           Rd6502(0x0100 + (byte) (R->S + 1)), 
           Rd6502(0x0100 + (byte) (R->S + 2)), 
           Rd6502(0x0100 + (byte) (R->S + 3)));
   showlastop(fp);
   //       fprintf(fp, "PPU: CR1: 0x%02X (NT:%d AI:%d SP:%d BP:%d SS:%d NMI:%d)\n",ppu.ControlRegister1.b, ppu.ControlRegister1.s.NameTblAddr, ppu.ControlRegister1.s.AddrIncrmt, ppu.ControlRegister1.s.SptPattern, ppu.ControlRegister1.s.BgPattern, ppu.ControlRegister1.s.SpriteSize, ppu.ControlRegister1.s.VBlank_NMI);
   //       fprintf(fp, "PPU: CR2: 0x%02X (FBC/CI:%d SV:%d BV:%d SC:%d BC:%d DT:%d)\n",ppu.ControlRegister2.b,ppu.ControlRegister2.s.Colour,ppu.ControlRegister2.s.SpriteVisibility,ppu.ControlRegister2.s.BgVisibility,ppu.ControlRegister2.s.SpriteClipping,ppu.ControlRegister2.s.BgClipping,ppu.ControlRegister2.s.DisplayType);
   //       fprintf(fp, "PPU: SR: 0x%02X (VB:%d S0:%d SSC:%d VWF:%d)\n", ppu.StatusRegister.b,ppu.StatusRegister.s.VBlankOccur,ppu.StatusRegister.s.Sprite0Occur,ppu.StatusRegister.s.SprtCount,ppu.StatusRegister.s.VRAMProtect);
   //       fprintf(fp, "PPU: M:%d ST:%d VRAMPtr:0x%04X T:0x%04X\n",ppu.MirrorDir,ppu.ScreenType,ppu.VRAMAddrReg2.W,ppu.TmpVRamPtr);
   
   //MapperDump(fp);
   
   for(I = 0; I < 0xFFFF; I += 0x10)
	 fprintf(fp, "%04X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X | %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
		    I,
		    Rd6502(I+0x00), Rd6502(I+0x01), Rd6502(I+0x02), Rd6502(I+0x03),
		    Rd6502(I+0x04), Rd6502(I+0x05), Rd6502(I+0x06), Rd6502(I+0x07),
		    Rd6502(I+0x08), Rd6502(I+0x09), Rd6502(I+0x0A), Rd6502(I+0x0B),
		    Rd6502(I+0x0C), Rd6502(I+0x0D), Rd6502(I+0x0E), Rd6502(I+0x0F),
		    // --- //
		    isprint(Rd6502(I+0x00))?Rd6502(I+0x00):'_',
		    isprint(Rd6502(I+0x01))?Rd6502(I+0x01):'_',
		    isprint(Rd6502(I+0x02))?Rd6502(I+0x02):'_',
		    isprint(Rd6502(I+0x03))?Rd6502(I+0x03):'_',
		    isprint(Rd6502(I+0x04))?Rd6502(I+0x04):'_',
		    isprint(Rd6502(I+0x05))?Rd6502(I+0x05):'_',
		    isprint(Rd6502(I+0x06))?Rd6502(I+0x06):'_',
		    isprint(Rd6502(I+0x07))?Rd6502(I+0x07):'_',
		    isprint(Rd6502(I+0x08))?Rd6502(I+0x08):'_',
		    isprint(Rd6502(I+0x09))?Rd6502(I+0x09):'_',
		    isprint(Rd6502(I+0x0A))?Rd6502(I+0x0A):'_',
		    isprint(Rd6502(I+0x0B))?Rd6502(I+0x0B):'_',
		    isprint(Rd6502(I+0x0C))?Rd6502(I+0x0C):'_',
		    isprint(Rd6502(I+0x0D))?Rd6502(I+0x0D):'_',
		    isprint(Rd6502(I+0x0E))?Rd6502(I+0x0E):'_',
		    isprint(Rd6502(I+0x0F))?Rd6502(I+0x0F):'_');
   
   DumpMemoryState(fp);
   
   fprintf(stderr, "\nPlease join this informations when submiting crash report\n");
   if (fp != stderr) fclose(fp);
   //getchar();
   exit(-42);
}

byte Page40[256];

void WrHook4000Multiplexer(byte addr, byte value)
{
   static byte SQ1V = 0;
   static byte SQ2V = 0;
   static byte NOIV = 0;
   
   static unsigned short SQ1P = 0;
   static unsigned short SQ2P = 0;
   static unsigned short TRIP = 0;
   static unsigned short NOIP = 0;
   
   static byte Sq1_reg0 = 0;
   static byte Sq1_reg1 = 0;
   static byte Sq1_reg2 = 0;
   static byte Sq1_reg3 = 0;
   
   static byte Sq2_reg0 = 0;
   static byte Sq2_reg1 = 0;
   static byte Sq2_reg2 = 0;
   static byte Sq2_reg3 = 0;
   
   double SQ = 0.0;
   
   switch(addr)
   {
#ifdef USE_SOUND    
	 case 0x00: /* DDLE NNNN */
	    Sq1_reg0 = value;
	    if (Sq1_reg0 & 0x10)
	    {
            SQ1V = 0x0F/*(0x04+(value&0x0F))& 0x0F*/;
	    }
	    else
	    {
            SQ1V = value&0x0F;
	    }
	    
	    break;
	    case 0x01: /* EPPP NSSS */        
	    Sq1_reg1 = value;
	    break;
	    case 0x02:
	    /*printf("Sq1 reg0: 0x%02X - duty:0x%X loop:0x%X env:0x%X vol:0x%X\n", 
		Sq1_reg0,
		(Sq1_reg0&0xC0)>>6,
		(Sq1_reg0&0x20)>>5,
		(Sq1_reg0&0x10)>>4,
		Sq1_reg0&0x0F);
		printf("Sq1 reg1: 0x%02X - sweep:0x%X period:0x%X neg:0x%X shift:0x%X\n", 
		Sq1_reg1,
		(Sq1_reg1&0x80)>>8,
		(Sq1_reg1&0x70)>>4,
		(Sq1_reg1&0x08)>>3,
		Sq1_reg1&0x07);
		printf("Sq1 reg2: 0x%02X\n", value);               
		printf("Sq1 reg3: 0x%02X\n", Sq1_reg3);*/
	    SQ1P = value | ((Sq1_reg3&0x7) << 8);
	    SQ = APU_BASEFREQ * 1000 * 1000 / (SQ1P+1 /*+ 
		(Sq1_reg1&0x80)?0:( (Sq1_reg1&0x08)?(SQ1P>>(Sq1_reg1&0x07)):(SQ1P<<(Sq1_reg1&0x07)) )*/);
	    SetSound(0,SND_MELODIC);
	    
	    //printf("SQ1V = %d - SQ = %f - SQ1P = %d\n", SQ1V, SQ, SQ1P);
	    
#ifdef SOUND_LOG
	 { FILE *fp = fopen("sound.log", "at"); fprintf(fp, "%d %d %d\n", 0, SQ1P, SQ1V); fclose(fp); }
#endif
	    Sound(0, (int) SQ/22, (0xFF/0x0F) * SQ1V);
	    
	    //        printf("40%02X: 0x%02X (SQ1P:%d SQ:%f (%d))\n", addr, value, SQ1P, SQ, (int) SQ);
	    Sq1_reg2 = value;
	    break;
	    
	    case 0x03:
	    Sq1_reg3 = value;
	    SQ1P = Sq1_reg2 | ((value&0x7) << 8);
	    SQ = APU_BASEFREQ * 1000 * 1000 / (SQ1P+1 /*+ 
		(Sq1_reg1&0x80)?0:( (Sq1_reg1&0x08)?(SQ1P>>(Sq1_reg1&0x07)):(SQ1P<<(Sq1_reg1&0x07)) )*/);            
#ifdef SOUND_LOG
	 { FILE *fp = fopen("sound.log", "at"); fprintf(fp, "%d %d %d\n", 0, SQ1P, SQ1V); fclose(fp); }
#endif
	    Sound(0, (int) SQ/22, (0xFF/0x0F) * SQ1V);      
	    break;
	    
	    
	    
	    case 0x04: 
	    Sq2_reg0 = value;
	    if (Sq2_reg0 & 0x10)
	    {
            SQ2V = 0x0F;
            //SQ2V = (0x04+(value&0x0F))& 0x0F;
	    }
	    else
	    {
		  SQ2V = value&0x0F;        
	    }
	    
	    break;
	    case 0x05:
	    Sq2_reg1 = value;
	    break;
	    
	    case 0x06:
	    Sq2_reg2 = value;
	    
	    SQ2P = Sq2_reg2 | ((Sq2_reg3&0x7) << 8);
	    
	    SQ = APU_BASEFREQ * 1000 * 1000 / (SQ2P+1 /*+ 
		(Sq2_reg1&0x80)?0:( (Sq2_reg1&0x08)?(SQ2P>>(Sq2_reg1&0x07)):(SQ2P<<(Sq2_reg1&0x07)) )*/);
	    
	    /*       printf("Sq2 reg0: 0x%02X - duty:0x%X loop:0x%X env:0x%X vol:0x%X\n", 
		Sq2_reg0,
		(Sq2_reg0&0xC0)>>6,
		(Sq2_reg0&0x20)>>5,
		(Sq2_reg0&0x10)>>4,
		Sq2_reg0&0x0F);
		printf("Sq2 reg1: 0x%02X - sweep:0x%X period:0x%X neg:0x%X shift:0x%X\n", 
		Sq2_reg1,
		(Sq2_reg1&0x80)>>8,
		(Sq2_reg1&0x70)>>4,
		(Sq2_reg1&0x08)>>3,
		Sq2_reg1&0x07);
		printf("Sq2 reg2: 0x%02X\n", value);               
		printf("Sq2 reg3: 0x%02X\n", Sq2_reg3);
		printf("SQ2V = %d - SQ = %f - SQ2P = %d\n", SQ2V, SQ, SQ2P);*/
#ifdef SOUND_LOG
	 { FILE *fp = fopen("sound.log", "at"); fprintf(fp, "%d %d %d\n", 1, SQ2P, SQ2V); fclose(fp); }
#endif
	    Sound(1, (int) SQ/22, (0xFF/0x0F) * SQ2V);
	    break;
	    
	    case 0x07:
	    Sq2_reg3 = value;
	    
	    SQ2P = Sq2_reg2 | ((Sq2_reg3&0x7) << 8);
	    //SQ2P = (SQ2P & 0x00FF) | ((value&0x7) << 8);
	    SQ = APU_BASEFREQ * 1000 * 1000 / (SQ2P+1 /*+ 
		(Sq2_reg1&0x80)?0:( (Sq2_reg1&0x08)?(SQ2P>>(Sq2_reg1&0x07)):(SQ2P<<(Sq2_reg1&0x07)) )*/);
#ifdef SOUND_LOG
	 { FILE *fp = fopen("sound.log", "at"); fprintf(fp, "%d %d %d\n", 1, SQ2P, SQ2V); fclose(fp); }
#endif
	    Sound(1, (int) SQ/22, (0xFF/0x0F) * SQ2V);        
	    break;
	    
	    case 0x0A:
	    TRIP = (TRIP & 0xFF00) | value;
	    SQ = APU_BASEFREQ * 1000 * 1000 / TRIP;
#ifdef SOUND_LOG
	 { FILE *fp = fopen("sound.log", "at"); fprintf(fp, "%d %d %d\n", 2, TRIP, 127); fclose(fp); }
#endif
	    Sound(2, (int) SQ/22, 127);
	    break;
	    
	    case 0x0B:
	    TRIP = (TRIP & 0x00FF) | ((value&0x7) << 8);;
	    SQ = APU_BASEFREQ * 1000 * 1000 / TRIP;
#ifdef SOUND_LOG
	 { FILE *fp = fopen("sound.log", "at"); fprintf(fp, "%d %d %d\n", 2, TRIP, 127); fclose(fp); }
#endif
	    Sound(2, (int) SQ/22, 127);
	    break;
	    
	    case 0x0C:
	    NOIV = value & 0x0F;
#ifdef SOUND_LOG
	 { FILE *fp = fopen("sound.log", "at"); fprintf(fp, "%d %d %d\n", 3, NOIP, NOIV); fclose(fp); }
#endif
	    SetSound(3, SND_NOISE);
	    Sound(3, (int) SQ/22, (0xFF/0x0F) * NOIV);        
	    break;
	    
	    case 0x0E:
	    NOIP = value & 0x0F;
	    SQ = APU_BASEFREQ * 1000 * 1000 / NOIP;
#ifdef SOUND_LOG
	 { FILE *fp = fopen("sound.log", "at"); fprintf(fp, "%d %d %d\n", 3, NOIP, NOIV); fclose(fp); }
#endif
	    SetSound(3, SND_NOISE);
	    Sound(3, (int) SQ/22,     NOIV);
	    break;
	    case 0x0F:
	    
	    break;
	    case 0x15:
	    /* DMC, Noise, Triangle, Sq 2, Sq 1 */
	    //SetChannels(0, (value&0x01)?0x01:0);
	    /*        printf("40%02X: 0x%02X [%c%c%c%c%c]\n", addr, value,
		(value&0x10)?'d':'.',
		(value&0x08)?'n':'.',
		(value&0x04)?'t':'.',
		(value&0x02)?'2':'.',
		(value&0x01)?'1':'.');*/
	    
	    break; 
#endif   
	    case 0x14:
	    ppu_fillSprRamDMA(value);
	    break;
	    
	    case 0x16:
	    WritePaddle(&P1, value);
	    //WritePaddle(&P2, value);
	    break;
	    
	    case 0x17:
	    //        printf("40%02X: 0x%02X\n", addr, value);       
         if (value == 0x00)
            Int6502(&MainCPU,INT_IRQ);
	    
	    break;
	    //    default:
	    //Page40[addr] = value;
	    // printf("40%02X: 0x%02X\n", addr, value);       
	    //        printf("pAPU: 0x%X @ 0x40%X\n", value, addr);
   }
   
}

byte RdHook4000Multiplexer(byte addr)
{
   byte ret;
   switch(addr)
   {
	 case 0x16:
	    ret = ReadPaddle(&P1);
	    break;
	    
	 case 0x17:
	    ret = 0x40;
	    break;
	    
	 case 0x15:
	    ret = 0x1F;
	 default:
	    ret = 0x42;
   }   
   return ret;
}

void printUsage(int argc, char *argv[])
{
   printf("Usage : %s game.nes [-p number][-f][-b filename.pal][ filename.nes\n"
		"   -p: to add plugin 'number'\n"
		"   -f: to start in FDS mode\n"
		"   -d: to start directily into the debugguer\n"
		"   -b: to use palette file 'filename.pal'\n",
		argv[0]);
   exit(0);
}

int main(int argc, char *argv[])
{    
   int i;
   unsigned char j, k;
   unsigned char *MemoryPage;
   
   /* Here we will fill the memory */
   /*
    --------------------------------------- $10000
    Upper Bank of Cartridge ROM
    --------------------------------------- $C000
    Lower Bank of Cartridge ROM
    --------------------------------------- $8000
    Cartridge RAM (may be battery-backed)
    --------------------------------------- $6000
    Expansion Modules
    --------------------------------------- $5000
    Input/Output
    --------------------------------------- $2000
    2kB Internal RAM, mirrored 4 times
    --------------------------------------- $0000
    */
   
   /* Print the banner */
   printf("--------------------------------------------------------------------------------\n"
          "Welcome to TI-NESulator v%d.%d - by Godzil\n"
          "Copyright 2003-2008 TRAPIER Manoel (godzil@godzil.net)\n"
          "%s\n%s\n%s\n"
          "--------------------------------------------------------------------------------\n\n", 
          V_MAJOR,
          V_MINOR,
          VS_REVISION,
          VS_LASTCHANGEDDATE,
          VS_AUTHOR);
   
   printf("Install signal handlers...\t[");
   //    signal(SIGABRT,signalhandler);
   printf("A");
   //    signal(SIGILL,signalhandler);
   printf("I");
   /*signal(SIGINT,signalhandler);*/
   printf(".");
   //    signal(SIGSEGV,signalhandler);
   printf("S");
   //    signal(SIGTERM,signalhandler);
   printf("T]\n");
   
   /*  */
   printf("Initialize memory...\t\t");
   InitMemory();
   printf("[ OK ]\n");
   printf("Parsing parameters (%d)...\n", argc);
   /* Now we use a real argument parser ! */
   for(i = 1 ; (i < argc) && (argv[i][0]=='-'); i++)
   {
      switch(argv[i][1])
      {
         default: /* Option not recognized */
         case 'h': /* ask for help */
            printUsage(argc, argv);
            break;
            
         case 'p': 
            if (atoi(argv[i+1]) != 0)
            {
               printf("-Load plugin #%d...\n", atoi(argv[i+1]));
               if ( plugin_load(atoi(argv[i+1])) == -1)
               {
                  plugin_list();
                  exit(0);
               }
               i++;
            }
            else
            {
               plugin_list();
               exit(0);
            }
            break;
            
            case 'f':
            printf("-Start with fds!\n");
            START_WITH_FDS = 1;
            break;
            
            case 'd':
            printf("-Start with debug!\n");
            START_DEBUG = 1;
            break;
            
            case 'b':
            printf("-Palette file is %s\n", argv[i+1]);
            PALETTE_FILENAME = argv[i+1];
            i++;
            break;      
      }
      
   }
   
   CART_FILENAME = argv[argc-1];
   
   if (CART_FILENAME == NULL)
      printUsage(argc, argv);
   
   printf("Allocating 6502 memory\t\t");
   
   /* Allocating first 0x7FF memory */
   MemoryPage = (unsigned char *)malloc (0x800);
   set_page_ptr_2k(0,MemoryPage);
   for (i = 0; i < 0x08; i++)
   {
      set_page_readable(i, true);
      set_page_writeable(i, true);
   }
   
   /* Set ghost starting from 0x800 */
   set_page_ghost(0x08,true,0x00);
   set_page_ghost(0x09,true,0x01);
   set_page_ghost(0x0A,true,0x02);
   set_page_ghost(0x0B,true,0x03);
   set_page_ghost(0x0C,true,0x04);
   set_page_ghost(0x0D,true,0x05);
   set_page_ghost(0x0E,true,0x06);
   set_page_ghost(0x0F,true,0x07);
   
   /* Set ghost starting from 0x1000 */
   set_page_ghost(0x10,true,0x00);
   set_page_ghost(0x11,true,0x01);
   set_page_ghost(0x12,true,0x02);
   set_page_ghost(0x13,true,0x03);
   set_page_ghost(0x14,true,0x04);
   set_page_ghost(0x15,true,0x05);
   set_page_ghost(0x16,true,0x06);
   set_page_ghost(0x17,true,0x07);
   
   /* Set ghost starting from 0x1800 */
   set_page_ghost(0x18,true,0x00);
   set_page_ghost(0x19,true,0x01);
   set_page_ghost(0x1A,true,0x02);
   set_page_ghost(0x1B,true,0x03);
   set_page_ghost(0x1C,true,0x04);
   set_page_ghost(0x1D,true,0x05);
   set_page_ghost(0x1E,true,0x06);
   set_page_ghost(0x1F,true,0x07);
   
   /* Set 0x4000 registers */
   
   /* "hack" : only page $40 is used by multiple devices, we need to multiplexe
    it*/
   set_page_wr_hook(0x40, WrHook4000Multiplexer);
   set_page_rd_hook(0x40, RdHook4000Multiplexer);
   
   set_page_readable(0x40, true);
   set_page_writeable(0x40, true);
   
   /* Exp ROM : Nothing to do actually */
   
   /* SRAM (0x6000 : 0x2000 bytes ) */
   MemoryPage = (unsigned char *)malloc (0x2000);
   
   set_page_ptr_8k(0x60, MemoryPage);
   
   /* ROM ptr will be set by mapper */
   /* But we will set the readable bit */
   for (i = 0x80; i < 0x100; i++)
   {
      set_page_readable(i, true);
      set_page_writeable(i, false);
   }
   
   printf("[ OK ]\n");
   
#define Value(s) (((s%0xFF) + (rand()%0xFF-128) )%0xFF)
   
   printf("Testing memory validity...\n");
   
   map_sram();
   
   for(i = 0x0000; i < 0x2000; i ++)
   {    
      j = Value(i);    
      MemoryPage[i] = j;
   }
   
   for(i = 0x6000; i < 0x8000; i ++)
   {    
      if (MemoryPage[i-0x6000] != (k = Rd6502(i)))
         printf("Error RdRead @ 0x%X [should:%d,is:%d]\n", i, MemoryPage[i-0x6000], k);
      if (MemoryPage[i-0x6000] != (k = Op6502(i)))
         printf("Error OpRead @ 0x%X [should:%d,is:%d]\n", i, MemoryPage[i-0x6000], k);
   }    
   
   printf("Testing memory... (<0x2000)\n");
   for( i = 0 ; i < 0x2000 ; i++ ) {
      j = Value(i);
      Wr6502(i, j);
      if ((k=Rd6502(i)) != j)
         printf("Error read/write @ 0x%X [w:%d,r:%d]\n", i, j, k);
      if ((k=Op6502(i)) != j)
         printf("Error opcode @ 0x%X [w:%d,r:%d]\n", i, j, k);
   }
   
   printf("Testing memory... (0x6000-0x8000)\n");
   for(i=0x6000;i<0x8000;i++) {
      j = Value(i);
      Wr6502(i, j);
      if ((k=Rd6502(i)) != j)
         printf("Error read/write @ 0x%X [w:%d,r:%d]\n", i, j, k);
      if ((k=Op6502(i)) != j)
         printf("Error opcode @ 0x%X [w:%d,r:%d]\n", i, j, k);
   }
   
   printf("Reseting main RAM...\t\t");
   /* Force the stack to be full of zero */
   for( i = 0x100 ; i < 0x200 ; i++ ) {
      Wr6502(i, 0x00);
   }
   
   
   for( i = 0x000 ; i < 0x800 ; i++ ) {
      Wr6502(i, 0x00);
   }
   
   printf("[ OK ]\n");
   
   if (START_WITH_FDS)
   {
      int fd;
      printf("Loading FDS ROM...\t\t");
      
      fd = open("../data/disksys.rom", O_RDONLY);
      
      FDSRom = mmap(NULL, 8*1024, PROT_READ, MAP_PRIVATE, fd, 0);
      printf("%p [ OK ]\n", FDSRom);
      close(fd);
      
      set_page_ptr_8k(0xE0, FDSRom);
      
      printf("Allocating FDS RAM...\t\t");
      
      FDSRam = (byte*) malloc( (8+16) * 1024);
      
      if (FDSRam == NULL)
         fatal("Allocation error\n");
      
      for (i = 0x80; i < 0xE0; i++)
      {
         set_page_ptr(i, FDSRam + (i*0x100));
         set_page_readable(i, true);
         set_page_writeable(i, true);
      }
      
      Cart->MapperID = 100;
   }
   else
   {
      Cart = malloc( sizeof (NesCart));
      if (Cart == NULL)
         fatal("Memory allocation error...\n");
      printf("Please Wait while loading %s cartridge...\n", CART_FILENAME);
      if (LoadCart(CART_FILENAME, Cart) != 0)
         fatal("Loading error...\n");
      
      if (Cart->Flags & iNES_BATTERY)
      {
         LoadSaveRam(CART_FILENAME);
      }        
      
   }
   
   unmap_sram();
   
   InitPaddle(&P1);
   
   printf("Initializing Allegro...\t\t");
   allegro_init();
   install_timer();
   install_keyboard();
   printf("[ OK ]\n");    
   printf("Set graphic mode...\t\t");
   set_color_depth(8);
   set_gfx_mode(GFX_AUTODETECT_WINDOWED, 512 + 256, 480, 512 + 256, 480);
   Buffer = create_bitmap(512 + 256, 480);
   clear_to_color(Buffer, 0x0D);
   
   set_close_button_callback(CloseHook);
   set_window_title("TI-NESulator");
   
   printf("[ OK ]\n");
   
   printf("Init PPU...\n");
   
   if (ppu_init() != 0)
      fatal("PPU Initialisation error..\n");   
   
   /* DumpMemoryState(); */
   if (Cart->Flags & iNES_4SCREEN)
   {
      ppu_setScreenMode(PPU_SCMODE_FOURSC);
   }
   else
   {       
      ppu_setScreenMode(PPU_SCMODE_NORMAL);
      ppu_setMirroring((Cart->Flags & iNES_MIRROR)?PPU_MIRROR_VERTICAL:PPU_MIRROR_HORIZTAL);
   }
   
   printf("Init mapper...\t\t\t");
   if (mapper_init(Cart) == -1)
      return -1;
   printf("[ OK ]\n");
   
   if (PALETTE_FILENAME == NULL)
   {
      set_palette(basicPalette);
   }
   else
   {
      LoadPalette(PALETTE_FILENAME, pal);    
      set_palette(pal);
   }
   
   /*  for(i = 0; i < 256; i++)
    {
    r = (r * 64) / 255;
    v = (v * 64) / 255;
    b = (b * 64) / 255;
    
    pal[i].r = r;
    pal[i].g = v;
    pal[i].b = b;
    
    pal[i + 64].r = r;
    pal[i + 64].g = v;
    pal[i + 64].b = b;
    
    pal[i + 128].r = r;
    pal[i + 128].g = v;
    pal[i + 128].b = b;
    
    pal[i + 192].r = r;
    pal[i + 192].g = v;
    pal[i + 192].b = b;
    
    printf("       { 0x%02X, 0x%02X, 0x%02X, 0x%02X },\n",
    (basicPalette[i].r * 64) / 255,
    (basicPalette[i].g * 64) / 255,
    (basicPalette[i].b * 64) / 255,
    basicPalette[i].filler);
    printf("       { 0x%02X, 0x%02X, 0x%02X, 0x%02X },\n",
    pal[i].r,
    pal[i].g,
    pal[i].b,
    pal[i].filler);
    }
    
    exit(0);*/
   
#ifdef USE_SOUND
   InitSound(48000,!0);
   
   SetSound(0, SND_RECTANGLE);
   SetSound(1, SND_RECTANGLE);
   SetSound(2, SND_TRIANGLE);
   SetSound(3, SND_NOISE);
#endif
   
   
   /*   short val = 0xCE;
    
    while(1)
    {  
    
    Wr6502(0x4000, 0x74);
    Wr6502(0x4001, 0x68);
    Wr6502(0x4002, val & 0xFF);
    Wr6502(0x4003, 0x08 | ((val & 0x700) >> 8));
    
    
    if (key[KEY_UP])
    val++;
    if (key[KEY_DOWN])
    val--;   
    
    usleep(500);     
    }
    
    
    exit(0);*/
   
   printf("Press ESC to pause emulation and jump to debugguer\n");
   install_int(ips_fps_counter, 1000);
   ScanLine = 0;
   
   //Do a loop every HBlank
   MainCPU.IPeriod = HBLANK_TIME;
   
   Reset6502(&MainCPU);
   
   MainCPU.Trace = 0;
   
   if (START_DEBUG)
      MainCPU.Trace = 1;
   
   gettimeofday(&timeStart, NULL);
   
   Run6502(&MainCPU);
   
   if (Cart->Flags & iNES_BATTERY)
   {
      SaveSaveRam(CART_FILENAME);
   }
   return 0;
}
END_OF_MAIN()

/** Rd6502()/Wr6502/Op6502() *********************************/
/** These functions are called when access to RAM occurs.   **/
/** They allow to control memory access. Op6502 is the same **/
/** as Rd6502, but used to read *opcodes* only, when many   **/
/** checks can be skipped to make it fast. It is only       **/
/** required if there is a #define FAST_RDOP.               **/
/************************************ TO BE WRITTEN BY USER **/
void Wr6502(register word Addr, register byte Value)
{            /* Write to memory */
   WriteMemory((Addr&0xFF00)>>8,Addr&0x00FF, Value);
}

byte Rd6502(register word Addr)
{            /* Read memory for normal use */
   return ReadMemory((Addr&0xFF00)>>8,Addr&0x00FF);
   
}

extern byte *memory_pages[0xFF];
byte Op6502(register word Addr)
{            /* Read OpCodes */
   byte *ptr;
   return ((ptr = memory_pages[(Addr&0xFF00)>>8])>(byte*)1)?ptr[Addr&0x00FF]:0;
   
   //return ReadMemory((Addr&0xFF00)>>8,Addr&0x00FF);
}

/** Loop6502() ***********************************************/
/** 6502 emulation calls this function periodically to      **/
/** check if the system hardware requires any interrupts.   **/
/** This function must return one of following values:      **/
/** INT_NONE, INT_IRQ, INT_NMI, or INT_QUIT to exit the     **/
/** emulation loop.                                         **/
/************************************ TO BE WRITTEN BY USER **/
byte Loop6502(register M6502 * R)
{
   byte ret;
   short skey; 
   long WaitTime;
   static long delta=0;
   
   ret = INT_NONE;
   
   if ( mapper_irqloop (ScanLine) )
   {
      ret = INT_IRQ;
      IRQScanHit = ScanLine;
   }
   
   if ( MapperWantIRQ == 1)
   {
      MapperWantIRQ = 0;
      ret = INT_IRQ;
   }
   
   if ( ppu_hblank(ScanLine) != 0 )
   {
      /*        if (ret == INT_IRQ)
       MapperWantIRQ = 1;*/
      ret = INT_NMI;        
   }
   
   if (ScanLine == 241)
      frame++;  
   
   if (ScanLine >= (240 + VBLANK_TIME - 1))
   {   /* End of VBlank Time */
      /* Sync at 60FPS */
      /* Get current time in microseconds */
      gettimeofday(&timeEnd, NULL);     
      
      WaitTime = (timeEnd.tv_sec) - (timeStart.tv_sec);           
      WaitTime *= 1000000;
      WaitTime += (timeEnd.tv_usec - timeStart.tv_usec);
#if !ISPAL && ISNTSC
      /* Calculate the waiting time, 16666 is the time of one frame in microseconds at a 60Hz rate) */
      WaitTime = 16666 - WaitTime + delta;
#elif ISPAL && !ISNTSC
      WaitTime = 20000 - WaitTime + delta;
#endif
      
      /* If we press Page Up, we dont we to accelerate "time" */
      if (!key[KEY_PGUP])
	  if ((WaitTime >= 0) && (WaitTime < 100000))
	  usleep(WaitTime);
      //usleep(WaitTime<0?0:(WaitTime>100000?0:WaitTime));
      
      /* Now get the time after sleep */
      gettimeofday(&timeStart, NULL);
      
      /* Now calculate How many microseconds we really spend in sleep and 
       calculate a delta for next iteration */
      delta = (timeStart.tv_sec) - (timeEnd.tv_sec);           
      delta *= 1000000;
      delta += (timeStart.tv_usec - timeEnd.tv_usec);
      delta = WaitTime - delta;
      
      /* To avoid strange time warp when stoping emulation or using acceleration a lot */
      if ((delta > 10000) || (delta < -10000)) 
         delta = 0;
      
      ScanLine = 0;
   }
   else
      
      ScanLine++;
   
   if (keypressed())
   {
      skey = (readkey() & 0xFF);
      if (skey == 27)
         R->Trace = 1;
      
      if (skey == '9')
      {
         VBLANK_TIME += 1;
         printf("VBLT: %d\n", VBLANK_TIME);
      }
      
      if (skey == '6')
      {
         VBLANK_TIME -= 1;
         printf("VBLT: %d\n", VBLANK_TIME);            
      }
      
      if (skey == '7')
      {
         HBLANK_TIME += 1;
         printf("HBLT: %d\n", HBLANK_TIME);  
         MainCPU.IPeriod = HBLANK_TIME;          
      }
      
      if (skey == '4')
      {
         HBLANK_TIME -= 1;
         printf("HBLT: %d\n", HBLANK_TIME);            
         MainCPU.IPeriod = HBLANK_TIME;
      }
      
      //        if ((skey == '&') || (skey == '1'))
      //          ppu.ForceBgVisibility = ~ppu.ForceBgVisibility;
      //        if ((skey == '¬™') || (skey == '2'))
      //          ppu.ForceSpriteVisibility = ~ppu.ForceSpriteVisibility;
      
      //        if ((skey == '"') || (skey == '3'))
      //          ppu.DisplayNameTables = ~ppu.DisplayNameTables;
      
      //        if ((skey == '\'') || (skey == '4'))
      //          ppu.DisplayAttributeTable = ~ppu.DisplayAttributeTable;
      
      //        if ((skey == '(') || (skey == '5'))
      //          ppu.DisplayPalette = ~ppu.DisplayPalette;
      
      //        if ((skey == '-') || (skey == 'Ô¨Ç') || (skey == '6'))        
      //          ppu.DisplayVRAM = ~ppu.DisplayVRAM;
      
      if ((skey == 'r') || (skey == 'R'))
      {
         //Reset6502(R);
         MainCPU.PC.B.l=Rd6502(0xFFFC);
         MainCPU.PC.B.h=Rd6502(0xFFFD);   
         
      }
      
      plugin_keypress(skey);
      
   }
   
   if (WantClosing == 1)
   {
      ret = INT_QUIT;
   }
   return ret;
}

/** Patch6502() **********************************************/
/** Emulation calls this function when it encounters an     **/
/** unknown opcode. This can be used to patch the code to   **/
/** emulate BIOS calls, such as disk and tape access. The   **/
/** function should return 1 if the exception was handled,  **/
/** or 0 if the opcode was truly illegal.                   **/
/************************************ TO BE WRITTEN BY USER **/
byte Patch6502(register byte Op, register M6502 * R)
{
   //printf("Invalid Opcode : 0x%X @ 0x%04X !\n", Op, R->PC.W);
   return 1;
}
