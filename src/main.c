/*
 *  Main application source file - The peTI-NESulator Project
 *  main.c
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

/* System includes */

#if !defined(__TIGCC__) && !defined(__GCC4TI__) && !defined(__GTC__)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>

#include <GLFW/glfw3.h>

#else

#define TIGCC_COMPAT
#include <tigcclib.h>

#endif

/* peTI-NESulator modules includes */
#include <os_dependent.h>

#include <corecpu.h>
#include <ppu/ppu.h>
#include <NESCarts.h>
#include <paddle.h>

#include <mappers/manager.h>

#include <memory/manager.h>

#include <plugins/manager.h>

#ifdef USE_SOUND
#include <Sound.h>
#endif

#if ISPAL && !ISNTSC
int VBLANK_TIME     = 70;
int HBLANK_TIME     = 103;
double APU_BASEFREQ = 1.7734474;
#elif !ISPAL && ISNTSC
int VBLANK_TIME     = 20;
int HBLANK_TIME     = 113;
double APU_BASEFREQ = 1.7897725;
#elif !ISPAL && !ISNTSC
# error You MUST define one of ISPAL Xor ISNTSC
#else
# error Cannot use ISPAL with ISNTSC together !
#endif

//#define MEMORY_TEST

/* peTI-NESulator Version */
#if !defined(V_MAJOR) || !defined(V_MINOR) || !defined(V_MICRO)
#error Something wrong with your building tools
#endif

#ifndef V_TEXT
#define V_TEXT ""
#endif



#ifdef USE_SOUND
#undef USE_SOUND
#endif

/* SVN specific values */

#define VS_REVISION        "$Revision$"
#define VS_LASTCHANGEDDATE "$LastChangedDate$"
#define VS_AUTHOR          "$Author$"

/*
#define MAXLASTOP 42

word latestop[MAXLASTOP];
*/

/* NES specific variables */
quick6502_cpu *MainCPU;
NesCart *Cart;

byte *FDSRom;
byte *FDSRam;

/* Command line options */
byte START_DEBUG = 0;
byte START_WITH_FDS = 0;
char *CART_FILENAME = NULL;
char *PALETTE_FILENAME = NULL;

Paddle P1, P2;

unsigned short ScanLine;

volatile int frame = 0;
volatile int ccount;

char MapperWantIRQ = 0;

char WantClosing = 0;

struct timeval timeStart;
struct timeval timeEnd;

volatile unsigned long FPS, IPS;

short IRQScanHit = -1;
short SZHit = -1;

/* palette */
unsigned long ColorPalette[ 8 * 63 ];

#define SET_RGB(r,g,b) ((((r<<8)|g)<<8)|b)|0xFF000000

/* Memory functions */
byte MemoryRead            (unsigned short Addr);
byte MemoryOpCodeRead      (unsigned short Addr);
byte MemoryStackRead       (unsigned short Addr);
byte MemoryPageZeroRead    (unsigned short Addr);

void MemoryWrite           (unsigned short Addr, byte Value);
void MemoryStackWrite      (unsigned short Addr, byte Value);
void MemoryPageZeroWrite   (unsigned short Addr, byte Value);

void Loop6502(quick6502_cpu *R);

void CloseHook(void)
{
   WantClosing = 1;
}

void SaveSaveRam(char *name)
{
   FILE *fp;
   int i;
   char fname[512];

   strcpy(fname, name);
   strcat(fname, ".svt");
   if ((fp = fopen(fname, "wb")))
   {
	 console_printf(Console_Default, "Saving savestate '%s'\n", fname);
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
	 console_printf(Console_Default, "Loading savestate '%s'\n", fname);
	 for( i = 0x60; i < 0x80; i++)
	 {
	    fread(get_page_ptr(i), 1, 0x0100, fp);
	 }    
	 fclose(fp);
	 
   }
}


void LoadPalette(char *filename, Palette *pal)
{
   FILE *fp;
   unsigned char r, v, b, i;
   console_printf(Console_Default, "%s: try to load pallette file '%s'", __func__, filename);
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
	    ColorPalette[i + (7 * 63)] = SET_RGB(r + 00, v + 00, b + 00);
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
	 console_printf(Console_Default, " [ OK ]\n");
   }
   else
   {
	 console_printf(Console_Error, "Error loading palette '%s'!\n", filename);
	 exit(-1);
   }
}

#ifdef RUN_COVERAGE
void alarmHandler(int sig)
{
   signal(SIGALRM, SIG_IGN);
   WantClosing = 1;
}
#endif

void signalhandler(int sig)
{
   static int state=0;

   char name[512];

   static FILE *fp = NULL;
   sprintf(name, "crashdump-%d.txt", (int)time(NULL));
   if (state != 0)
   {
	 console_printf(Console_Error, "\n\n\nCrashed within signal!\nEmergency exit\n");
	 exit(42);
   }
   state = 1;
   
   if (fp == NULL)
	 fp = fopen(name, "wt");
   
   state = 2;
   
   if (fp) console_printf(Console_Error, 
			    "\n\n\n\n\n"
			    "#sick# peTI-NESulator %d.%d.%d%s #sick#\n"
			    "see %s for more information",
			    V_MAJOR, V_MINOR, V_MICRO, V_TEXT,
			    name);
   
   if (!fp) fp = stderr;
   
   fprintf(fp,"\n\n\n\n\n"
		 "#sick# peTI-NESulator %d.%d.%d%s #sick# signal: ",
		 V_MAJOR, V_MINOR, V_MICRO, V_TEXT);
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

   //quick6502_dump(cpu, fp);

   //showlastop(fp);
   //       fprintf(fp, "PPU: CR1: 0x%02X (NT:%d AI:%d SP:%d BP:%d SS:%d NMI:%d)\n",ppu.ControlRegister1.b, ppu.ControlRegister1.s.NameTblAddr, ppu.ControlRegister1.s.AddrIncrmt, ppu.ControlRegister1.s.SptPattern, ppu.ControlRegister1.s.BgPattern, ppu.ControlRegister1.s.SpriteSize, ppu.ControlRegister1.s.VBlank_NMI);
   //       fprintf(fp, "PPU: CR2: 0x%02X (FBC/CI:%d SV:%d BV:%d SC:%d BC:%d DT:%d)\n",ppu.ControlRegister2.b,ppu.ControlRegister2.s.Colour,ppu.ControlRegister2.s.SpriteVisibility,ppu.ControlRegister2.s.BgVisibility,ppu.ControlRegister2.s.SpriteClipping,ppu.ControlRegister2.s.BgClipping,ppu.ControlRegister2.s.DisplayType);
   //       fprintf(fp, "PPU: SR: 0x%02X (VB:%d S0:%d SSC:%d VWF:%d)\n", ppu.StatusRegister.b,ppu.StatusRegister.s.VBlankOccur,ppu.StatusRegister.s.Sprite0Occur,ppu.StatusRegister.s.SprtCount,ppu.StatusRegister.s.VRAMProtect);
   //       fprintf(fp, "PPU: M:%d ST:%d VRAMPtr:0x%04X T:0x%04X\n",ppu.MirrorDir,ppu.ScreenType,ppu.VRAMAddrReg2.W,ppu.TmpVRamPtr);
   
   //MapperDump(fp);
#if 0   
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
#endif   
   DumpMemoryState(fp);
   
   console_printf(Console_Error, "\nPlease join this informations when submiting crash report\n");
   if (fp != stderr) fclose(fp);

   exit(-42);
}

byte Page40[256];

void WrHook4000Multiplexer(byte addr, byte value)
{
#ifdef USE_SOUND
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
#endif

   switch(addr)
   {
#ifdef USE_SOUND    
	 case 0x00: /* DDLE NNNN */
	    Sq1_reg0 = value;
	    if (Sq1_reg0 & 0x10)
	    {
            SQ1V = (0x04+(value&0x0F))& 0x0F;
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
	    /*console_printf(Console_Default, "Sq1 reg0: 0x%02X - duty:0x%X loop:0x%X env:0x%X vol:0x%X\n", 
		Sq1_reg0,
		(Sq1_reg0&0xC0)>>6,
		(Sq1_reg0&0x20)>>5,
		(Sq1_reg0&0x10)>>4,
		Sq1_reg0&0x0F);
		console_printf(Console_Default, "Sq1 reg1: 0x%02X - sweep:0x%X period:0x%X neg:0x%X shift:0x%X\n", 
		Sq1_reg1,
		(Sq1_reg1&0x80)>>8,
		(Sq1_reg1&0x70)>>4,
		(Sq1_reg1&0x08)>>3,
		Sq1_reg1&0x07);
		console_printf(Console_Default, "Sq1 reg2: 0x%02X\n", value);               
		console_printf(Console_Default, "Sq1 reg3: 0x%02X\n", Sq1_reg3);*/
	    SQ1P = value | ((Sq1_reg3&0x7) << 8);
	    SQ = APU_BASEFREQ * 1000 * 1000 / (SQ1P+1 /*+ 
		(Sq1_reg1&0x80)?0:( (Sq1_reg1&0x08)?(SQ1P>>(Sq1_reg1&0x07)):(SQ1P<<(Sq1_reg1&0x07)) )*/);
	    //SetSound(0,SND_MELODIC);
	    
	    //console_printf(Console_Default, "SQ1V = %d - SQ = %f - SQ1P = %d\n", SQ1V, SQ, SQ1P);
	    
#ifdef SOUND_LOG
	 { FILE *fp = fopen("sound.log", "at"); fprintf(fp, "%d %d %d\n", 0, SQ1P, SQ1V); fclose(fp); }
#endif
	    Sound(0, (int) SQ/22, (0xFF/0x0F) * SQ1V);
	    
         //console_printf(Console_Default, "40%02X: 0x%02X (SQ1P:%d SQ:%f (%d))\n", addr, value, SQ1P, SQ, (int) SQ);
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
            SQ2V = (0x04+(value&0x0F))& 0x0F;
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
	    
	    /*       console_printf(Console_Default, "Sq2 reg0: 0x%02X - duty:0x%X loop:0x%X env:0x%X vol:0x%X\n", 
		Sq2_reg0,
		(Sq2_reg0&0xC0)>>6,
		(Sq2_reg0&0x20)>>5,
		(Sq2_reg0&0x10)>>4,
		Sq2_reg0&0x0F);
		console_printf(Console_Default, "Sq2 reg1: 0x%02X - sweep:0x%X period:0x%X neg:0x%X shift:0x%X\n", 
		Sq2_reg1,
		(Sq2_reg1&0x80)>>8,
		(Sq2_reg1&0x70)>>4,
		(Sq2_reg1&0x08)>>3,
		Sq2_reg1&0x07);
		console_printf(Console_Default, "Sq2 reg2: 0x%02X\n", value);               
		console_printf(Console_Default, "Sq2 reg3: 0x%02X\n", Sq2_reg3);
		console_printf(Console_Default, "SQ2V = %d - SQ = %f - SQ2P = %d\n", SQ2V, SQ, SQ2P);*/
#ifdef SOUND_LOG
	 { FILE *fp = fopen("sound.log", "at"); fprintf(fp, "%d %d %d\n", 1, SQ2P, SQ2V); fclose(fp); }
#endif
	    Sound(1, (int) SQ/22, (0xFF/0x0F) * SQ2V);
	    break;
	    
	    case 0x07:
	    Sq2_reg3 = value;
	    
	    SQ2P = Sq2_reg2 | ((Sq2_reg3&0x7) << 8);
	    
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
	    //SetSound(3, SND_NOISE);
	    Sound(3, (int) SQ/22, (0xFF/0x0F) * NOIV);        
	    break;
	    
	    case 0x0E:
	    NOIP = value & 0x0F;
	    SQ = APU_BASEFREQ * 1000 * 1000 / NOIP;
#ifdef SOUND_LOG
	 { FILE *fp = fopen("sound.log", "at"); fprintf(fp, "%d %d %d\n", 3, NOIP, NOIV); fclose(fp); }
#endif
	    //SetSound(3, SND_NOISE);
	    Sound(3, (int) SQ/22,     NOIV);
	    break;
	    case 0x0F:
	    
	    break;
	    case 0x15:
	    /* DMC, Noise, Triangle, Sq 2, Sq 1 */
	    //SetChannels(0, (value&0x01)?0x01:0);
	    /*        console_printf(Console_Default, "40%02X: 0x%02X [%c%c%c%c%c]\n", addr, value,
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
            if (value == 0x00)
            {
               quick6502_int(MainCPU, Q6502_IRQ_SIGNAL);
            }
	    
            break;

        default:
	      Page40[addr] = value;
	    // console_printf(Console_Default, "40%02X: 0x%02X\n", addr, value);       
	    //        console_printf(Console_Default, "pAPU: 0x%X @ 0x40%X\n", value, addr);
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
   console_printf(Console_Default, "Usage : %s game.nes [-p number][-f][-b filename.pal][ filename.nes\n"
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
   unsigned char *MemoryPage;
   quick6502_cpuconfig CpuConfig;
   
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
   console_init(Console_Debug);
   /* Print the banner */
   console_printf(Console_Default, "--------------------------------------------------------------------------------\n"
          "Welcome to peTI-NESulator v%d.%d.%d%s - by Godzil`\n"
          "Copyright 2003-2018 Manoel TRAPIER (petines@godzil.net)\n"
          "--------------------------------------------------------------------------------\n\n", 
          V_MAJOR, V_MINOR, V_MICRO, V_TEXT);
   
   console_printf(Console_Default, "Install signal handlers...\t[");
   signal(SIGABRT, signalhandler);
   console_printf(Console_Default, "A");
   signal(SIGILL, signalhandler);
   console_printf(Console_Default, "I");
   /*signal(SIGINT, signalhandler);*/
   console_printf(Console_Default, ".");
   signal(SIGSEGV, signalhandler);
   console_printf(Console_Default, "S");
   signal(SIGTERM, signalhandler);
   console_printf(Console_Default, "T]\n");

#ifdef RUN_COVERAGE
   signal(SIGALRM, alarmHandler);
#endif
  
   /*  */
   console_printf(Console_Default, "Initialize memory...\t\t");
   InitMemory();
   console_printf(Console_Default, "[ OK ]\n");
   console_printf(Console_Default, "Parsing parameters (%d)...\n", argc);
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
               console_printf(Console_Default, "-Load plugin #%d...\n", atoi(argv[i+1]));
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
            console_printf(Console_Default, "-Start with fds!\n");
            START_WITH_FDS = 1;
            break;
            
            case 'd':
            console_printf(Console_Default, "-Start with debug!\n");
            START_DEBUG = 1;
            break;
            
            case 'b':
            console_printf(Console_Default, "-Palette file is %s\n", argv[i+1]);
            PALETTE_FILENAME = argv[i+1];
            i++;
            break;      
      }
      
   }
   
   CART_FILENAME = argv[argc-1];
   
   if (CART_FILENAME == NULL)
      printUsage(argc, argv);
   
   console_printf(Console_Default, "Allocating 6502 memory\t\t");
   
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
   
   /* ROM ptr will be set by mapper */
   /* But we will set the readable bit */
   for (i = 0x80; i < 0x100; i++)
   {
      set_page_readable(i, true);
      set_page_writeable(i, false);
   }
   
   console_printf(Console_Default, "[ OK ]\n");
   
#define Value(s) (((s%0xFF) + (rand()%0xFF-128) )%0xFF)
   
#ifdef MEMORY_TEST
   console_printf(Console_Default, "Testing memory validity...\n");
   
   map_sram();
   
   console_printf(Console_Verbose, "Testing Page Zero\n");
   for( i = 0 ; i < 0x100 ; i++)
   {
      j = rand() % 0xFF;
      MemoryPage[i] = j;
      if ((k = MemoryPageZeroRead(i)) != j)
         console_printf(Console_Error, "Error MemoryPageZeroRead  @ 0x%04X [j:%02X, should:%02X, is:%02X]\n", i, j, MemoryPage[i], k);
      
      j = rand() % 0xFF;
      MemoryPageZeroWrite(i, j);
      if ((k = MemoryPage[i]) != j)
         console_printf(Console_Error, "Error MemoryPageZeroWrite @ 0x%04X [j:%02X, should:%02X, is:%02X]\n", i, j, MemoryPage[i], k);
      MemoryPage[i] = 0;
   }
   
   console_printf(Console_Verbose, "Testing memory... (<0x2000)\n");
   for( i = 0 ; i < 0x2000 ; i++ )
   {
      j = Value(i);
      MemoryWrite(i, j);
      if ((k=MemoryRead(i)) != j)
         console_printf(Console_Error, "Error read/write @ 0x%X [w:%d,r:%d]\n", i, j, k);
      if ((k=MemoryOpCodeRead(i)) != j)
         console_printf(Console_Error, "Error opcode @ 0x%X [w:%d,r:%d]\n", i, j, k);
   }
#endif
   /* SRAM (0x6000 : 0x2000 bytes ) */
   MemoryPage = (unsigned char *)malloc (0x2000);
   
   set_page_ptr_8k(0x60, MemoryPage);
   
#ifdef MEMORY_TEST
   for(i = 0x6000; i < 0x8000; i ++)
   {    
      if (MemoryPage[i-0x6000] != (k = MemoryRead(i)))
         console_printf(Console_Error, "Error MemoryRead @ 0x%X [should:%d,is:%d]\n", i, MemoryPage[i-0x6000], k);
      if (MemoryPage[i-0x6000] != (k = MemoryOpCodeRead(i)))
         console_printf(Console_Error, "Error MemoryOpCodeRead @ 0x%X [should:%d,is:%d]\n", i, MemoryPage[i-0x6000], k);
   }    
   
   console_printf(Console_Verbose, "Testing memory... (0x6000-0x8000)\n");
   for(i=0x6000;i<0x8000;i++)
   {
      j = Value(i);
      MemoryWrite(i, j);
      if ((k=MemoryRead(i)) != j)
         console_printf(Console_Error, "Error read/write @ 0x%X [w:%d,r:%d]\n", i, j, k);
      if ((k=MemoryOpCodeRead(i)) != j)
         console_printf(Console_Error, "Error opcode @ 0x%X [w:%d,r:%d]\n", i, j, k);
   }

   console_printf(Console_Default, "Reseting main RAM...\t\t");

   /* Force the stack to be full of zero */
   for( i = 0x100 ; i < 0x200 ; i++ )
   {
      MemoryWrite(i, 0x00);
   }
   
   console_printf(Console_Default, "[ OK ]\n");
#endif
 
   Cart = malloc( sizeof (NesCart));
   if (Cart == NULL)
   {
      console_printf(Console_Error, "Memory allocation error...\n");
      exit(-1);
   }
   
   if (START_WITH_FDS)
   {
      int fd;
      console_printf(Console_Default, "Loading FDS ROM...\t\t");
	 fd = open("../data/disksys.rom", O_RDONLY);
	 //fd = open("peTI-NESulator.app/Contents/Resources/disksys.rom", O_RDONLY);
      if (fd < 0)
      {
         console_printf(Console_Error, "Can't find FDS ROM...\n");
         exit(-1);
      }
      
      FDSRom = mmap(NULL, 8*1024, PROT_READ, MAP_PRIVATE, fd, 0);
      console_printf(Console_Default, "%p [ OK ]\n", FDSRom);
      close(fd);
      
      set_page_ptr_8k(0xE0, FDSRom);
      
      console_printf(Console_Default, "Allocating FDS RAM...\n");
      
      FDSRam = (byte*) malloc( (8+16) * 1024);
      
      if (FDSRam == NULL)
      {
         console_printf(Console_Error, "Allocation error\n");
         exit(-1);
      }
      
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
      console_printf(Console_Default, "Please Wait while loading %s cartridge...\n", CART_FILENAME);
      if (LoadCart(CART_FILENAME, Cart) != 0)
      {
         console_printf(Console_Error, "Loading error...\n");
         exit(-1);
      }
      
      if (Cart->Flags & iNES_BATTERY)
      {
         LoadSaveRam(CART_FILENAME);
      }        
      
   }

   unmap_sram();
   
   InitPaddle(&P1);

   console_printf(Console_Default, "Init PPU...\n");
   
   if (ppu_init() != 0)
   {
      console_printf(Console_Error, "PPU Initialisation error..\n");
      exit(-1);
   }
   
   DumpMemoryState(stdout);
   
   if (Cart->Flags & iNES_4SCREEN)
   {
      ppu_setScreenMode(PPU_SCMODE_FOURSC);
   }
   else
   {       
      ppu_setScreenMode(PPU_SCMODE_NORMAL);
      ppu_setMirroring((Cart->Flags & iNES_MIRROR)?PPU_MIRROR_VERTICAL:PPU_MIRROR_HORIZTAL);
   }
   
   console_printf(Console_Default, "Init mapper...\t\t\t");
   if (mapper_init(Cart) == -1)
      return -1;
   console_printf(Console_Default, "[ OK ]\n");
   
//  set_palette(basicPalette);

#ifdef USE_SOUND
   InitSound(44400,!0);
   
   SetSound(0, SND_RECTANGLE);
   SetSound(1, SND_RECTANGLE);
   SetSound(2, SND_TRIANGLE);
   SetSound(3, SND_NOISE);
#endif
   // Actually no real debugguer...
   //console_printf(Console_Default, "Press ESC to pause emulation and jump to debugguer\n");

   ScanLine = 0;
   
   /* Initialize the CPU */
   CpuConfig.memory_read        = MemoryRead;
   CpuConfig.memory_write       = MemoryWrite;
   CpuConfig.memory_page0_read  = MemoryPageZeroRead;
   CpuConfig.memory_page0_write = MemoryPageZeroWrite;
   CpuConfig.memory_stack_read  = MemoryStackRead;
   CpuConfig.memory_stack_write = MemoryStackWrite;
   CpuConfig.memory_opcode_read = MemoryOpCodeRead;
   
   MainCPU = quick6502_init(&CpuConfig);
   
   quick6502_reset(MainCPU);
   
/* No debugger actually
   MainCPU.Trace = 0;
   if (START_DEBUG)
      MainCPU.Trace = 1;
 */
   
   gettimeofday(&timeStart, NULL);

#ifdef RUN_COVERAGE
   alarm(1 * 60); /* Run for 1 minutes */
#endif
   
   while(!WantClosing)
   {
      ccount += quick6502_run(MainCPU, HBLANK_TIME);

      Loop6502(MainCPU);
   }
   
   if (Cart->Flags & iNES_BATTERY)
   {
      SaveSaveRam(CART_FILENAME);
   }
   return 0;
}

/* Access directly to Memory pages *HACKISH* */
extern byte *memory_pages[0xFF];
/* Memory functions */

/* Read memory, general function */
byte MemoryRead            (unsigned short Addr)
{            
   return ReadMemory((Addr&0xFF00)>>8,Addr&0x00FF);   
}

/* Read memory for opcode (need fast access) */
byte MemoryOpCodeRead      (unsigned short Addr)
{
   byte *ptr;
   return ((ptr = memory_pages[(Addr&0xFF00)>>8])>(byte*)1)?ptr[Addr&0x00FF]:0xEA;
}

byte MemoryStackRead       (unsigned short Addr)
{
   byte *ptr = memory_pages[1];
   return ptr[Addr&0x00FF];
}

byte MemoryPageZeroRead    (unsigned short Addr)
{
   byte *ptr = memory_pages[0];
   return ptr[Addr&0x00FF];
}

/* Write to memory, general function */
void MemoryWrite           (unsigned short Addr, byte Value)
{
   WriteMemory((Addr&0xFF00)>>8,Addr&0x00FF, Value);
}

void MemoryStackWrite      (unsigned short Addr, byte Value)
{
   byte *ptr = memory_pages[1];
   ptr[Addr&0x00FF] = Value;
}

void MemoryPageZeroWrite   (unsigned short Addr, byte Value)
{
   byte *ptr = memory_pages[0];
   ptr[Addr&0x00FF] = Value;
}

void Loop6502(quick6502_cpu *R)
{
   byte ret;
//   short skey;
   long WaitTime;
   static long delta=0;
   
   ret = 0;
   
   if ( (mapper_irqloop) && ( mapper_irqloop (ScanLine) ) )
   {
      ret = Q6502_IRQ_SIGNAL;
      IRQScanHit = ScanLine;
   }
   
   if ( MapperWantIRQ == 1)
   {
      MapperWantIRQ = 0;
      ret = Q6502_IRQ_SIGNAL;
   }
   
   if ( ppu_hblank(ScanLine) != 0 )
   {
      ret = Q6502_NMI_SIGNAL;        
   }
   
   if (ScanLine == (239 + VBLANK_TIME))
   {  /* End of VBlank Time */
      frame++;
      SZHit = -1;
      IRQScanHit = -1;
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
      
      /* If we press Page Up, we want to accelerate "time" */
      if (!getKeyStatus('Y'))
         if ((WaitTime >= 0) && (WaitTime < 100000))
	         usleep(WaitTime);
      
      /* Now get the time after sleep */
      gettimeofday(&timeStart, NULL);
      
      /* Now calculate How many microseconds we really spend in sleep and 
       calculate a delta for next iteration */
      delta = (timeStart.tv_sec) - (timeEnd.tv_sec);           
      delta *= 1000000;
      delta += (timeStart.tv_usec - timeEnd.tv_usec);
      delta = WaitTime - delta;

      /* To avoid strange time warp when stopping emulation or using acceleration a lot */
      if ((delta > 10000) || (delta < -10000)) 
         delta = 0;
   }

   /* There is Two dummy scanline */
   if (ScanLine >= (239 + VBLANK_TIME + 4))
      ScanLine = 0;
   else
      ScanLine++;
   
   //console_printf(Console_Default, "SL:%d HBT:%d VbT:%d\n", ScanLine, HBLANK_TIME, VBLANK_TIME);

      // TODO: NO DEBUGER
      if (getKeyStatus(GLFW_KEY_ESCAPE))
         exit(0);

#if 0
      if (skey == '9')
      {
         VBLANK_TIME += 2;
         console_printf(Console_Default, "VBLT: %d\n", VBLANK_TIME);
      }
      
      if (skey == '6')
      {
         VBLANK_TIME -= 2;
         console_printf(Console_Default, "VBLT: %d\n", VBLANK_TIME);            
      }
      
      if (skey == '7')
      {
         HBLANK_TIME += 1;
         console_printf(Console_Default, "HBLT: %d\n", HBLANK_TIME);  
      }
      
      if (skey == '4')
      {
         HBLANK_TIME -= 1;
         console_printf(Console_Default, "HBLT: %d\n", HBLANK_TIME);            
      }
#endif

      if (getKeyStatus('r') || getKeyStatus('R'))
      {
         /* Force the PPU to stop NMIs */
         MemoryWrite(0x2000, 0x00);
         quick6502_reset(R);
      }
      
//      plugin_keypress(skey);

   if (ret != 0)
      quick6502_int(R, ret);
}

