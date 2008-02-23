/** M6502: portable 6502 emulator ****************************/
/**                                                         **/
/**                         M6502.c                         **/
/**                                                         **/
/** This file contains implementation for 6502 CPU. Don't   **/
/** forget to provide Rd6502(), Wr6502(), Loop6502(), and   **/
/** possibly Op6502() functions to accomodate the emulated  **/
/** machine's architecture.                                 **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1996-2002                 **/
/**               Alex Krasivsky  1996                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/   
/**     changes to this file.                               **/
/*************************************************************/
/*
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 */
 
#include "M6502.h"
#include "Tables.h"
#include <stdio.h>

/** INLINE ***************************************************/
/** Different compilers inline C functions differently.     **/
/*************************************************************/
#ifdef __GNUC__
#define INLINE inline
#else
#define INLINE static
#endif

int icount = 0;

/** System-Dependent Stuff ***********************************/
/** This is system-dependent code put here to speed things  **/
/** up. It has to stay inlined to be fast.                  **/
/*************************************************************/
#ifdef INES
#define FAST_RDOP
extern byte *Page[];
INLINE byte Op6502(register word A) { return(Page[A>>13][A&0x1FFF]); }
#endif

/** FAST_RDOP ************************************************/
/** With this #define not present, Rd6502() should perform  **/
/** the functions of Rd6502().                              **/
/*************************************************************/
#ifndef FAST_RDOP
#define Op6502(A) Rd6502(A)
#endif

/** Addressing Methods ***************************************/
/** These macros calculate and return effective addresses.  **/
/*************************************************************/
#define MC_Ab(Rg)    M_LDWORD(Rg)
#define MC_Zp(Rg)       Rg.W=Op6502(R->PC.W++)
#define MC_Zx(Rg)       Rg.W=(byte)(Op6502(R->PC.W++)+R->X)
#define MC_Zy(Rg)       Rg.W=(byte)(Op6502(R->PC.W++)+R->Y)
#define MC_Ax(Rg)    M_LDWORD(Rg);Rg.W+=R->X
#define MC_Ay(Rg)    M_LDWORD(Rg);Rg.W+=R->Y
#define MC_Ix(Rg)       K.W=(byte)(Op6502(R->PC.W++)+R->X); \
            Rg.B.l=Op6502(K.W++);Rg.B.h=Op6502(K.W)
#define MC_Iy(Rg)       K.W=Op6502(R->PC.W++); \
            Rg.B.l=Op6502(K.W++);Rg.B.h=Op6502(K.W); \
            Rg.W+=R->Y

/** Reading From Memory **************************************/
/** These macros calculate address and read from it.        **/
/*************************************************************/
#define MR_Ab(Rg)    MC_Ab(J);Rg=Rd6502(J.W)
#define MR_Im(Rg)    Rg=Op6502(R->PC.W++)
#define    MR_Zp(Rg)    MC_Zp(J);Rg=Rd6502(J.W)
#define MR_Zx(Rg)    MC_Zx(J);Rg=Rd6502(J.W)
#define MR_Zy(Rg)    MC_Zy(J);Rg=Rd6502(J.W)
#define    MR_Ax(Rg)    MC_Ax(J);Rg=Rd6502(J.W)
#define MR_Ay(Rg)    MC_Ay(J);Rg=Rd6502(J.W)
#define MR_Ix(Rg)    MC_Ix(J);Rg=Rd6502(J.W)
#define MR_Iy(Rg)    MC_Iy(J);Rg=Rd6502(J.W)

/** Writing To Memory ****************************************/
/** These macros calculate address and write to it.         **/
/*************************************************************/
#define MW_Ab(Rg)    MC_Ab(J);Wr6502(J.W,Rg)
#define MW_Zp(Rg)    MC_Zp(J);Wr6502(J.W,Rg)
#define MW_Zx(Rg)    MC_Zx(J);Wr6502(J.W,Rg)
#define MW_Zy(Rg)    MC_Zy(J);Wr6502(J.W,Rg)
#define MW_Ax(Rg)    MC_Ax(J);Wr6502(J.W,Rg)
#define MW_Ay(Rg)    MC_Ay(J);Wr6502(J.W,Rg)
#define MW_Ix(Rg)    MC_Ix(J);Wr6502(J.W,Rg)
#define MW_Iy(Rg)    MC_Iy(J);Wr6502(J.W,Rg)

/** Modifying Memory *****************************************/
/** These macros calculate address and modify it.           **/
/*************************************************************/
#define MM_Ab(Cmd)    MC_Ab(J);I=Rd6502(J.W);Cmd(I);Wr6502(J.W,I)
#define MM_Zp(Cmd)    MC_Zp(J);I=Rd6502(J.W);Cmd(I);Wr6502(J.W,I)
#define MM_Zx(Cmd)    MC_Zx(J);I=Rd6502(J.W);Cmd(I);Wr6502(J.W,I)
#define MM_Ax(Cmd)    MC_Ax(J);I=Rd6502(J.W);Cmd(I);Wr6502(J.W,I)

/** Other Macros *********************************************/
/** Calculating flags, stack, jumps, arithmetics, etc.      **/
/*************************************************************/
#define M_FL(Rg)    R->P=(R->P&~(Z_FLAG|N_FLAG))|ZNTable[Rg]
#define M_LDWORD(Rg)    Rg.B.l=Op6502(R->PC.W++);Rg.B.h=Op6502(R->PC.W++)

#define M_PUSH(Rg)    Wr6502(0x0100|R->S,Rg);R->S--
#define M_POP(Rg)    R->S++;Rg=Op6502(0x0100|R->S)
#define M_JR        R->PC.W+=(offset)Op6502(R->PC.W)+1;R->ICount--

#ifdef NO_DECIMAL

#define M_ADC(Rg) \
  K.W=R->A+Rg+(R->P&C_FLAG); \
  R->P&=~(N_FLAG|V_FLAG|Z_FLAG|C_FLAG); \
  R->P|=(~(R->A^Rg)&(R->A^K.B.l)&0x80? V_FLAG:0)| \
        (K.B.h? C_FLAG:0)|ZNTable[K.B.l]; \
  R->A=K.B.l

/* Warning! C_FLAG is inverted before SBC and after it */
#define M_SBC(Rg) \
  K.W=R->A-Rg-(~R->P&C_FLAG); \
  R->P&=~(N_FLAG|V_FLAG|Z_FLAG|C_FLAG); \
  R->P|=((R->A^Rg)&(R->A^K.B.l)&0x80? V_FLAG:0)| \
        (K.B.h? 0:C_FLAG)|ZNTable[K.B.l]; \
  R->A=K.B.l

#else /* NO_DECIMAL */

#define M_ADC(Rg) \
  if(R->P&D_FLAG) \
  { \
    K.B.l=(R->A&0x0F)+(Rg&0x0F)+(R->P&C_FLAG); \
    if(K.B.l>9) K.B.l+=6; \
    K.B.h=(R->A>>4)+(Rg>>4)+(K.B.l>15? 1:0); \
    R->A=(K.B.l&0x0F)|(K.B.h<<4); \
    R->P=(R->P&~C_FLAG)|(K.B.h>15? C_FLAG:0); \
  } \
  else \
  { \
    K.W=R->A+Rg+(R->P&C_FLAG); \
    R->P&=~(N_FLAG|V_FLAG|Z_FLAG|C_FLAG); \
    R->P|=(~(R->A^Rg)&(R->A^K.B.l)&0x80? V_FLAG:0)| \
          (K.B.h? C_FLAG:0)|ZNTable[K.B.l]; \
    R->A=K.B.l; \
  }

/* Warning! C_FLAG is inverted before SBC and after it */
#define M_SBC(Rg) \
  if(R->P&D_FLAG) \
  { \
    K.B.l=(R->A&0x0F)-(Rg&0x0F)-(~R->P&C_FLAG); \
    if(K.B.l&0x10) K.B.l-=6; \
    K.B.h=(R->A>>4)-(Rg>>4)-((K.B.l&0x10)>>4); \
    if(K.B.h&0x10) K.B.h-=6; \
    R->A=(K.B.l&0x0F)|(K.B.h<<4); \
    R->P=(R->P&~C_FLAG)|(K.B.h>15? 0:C_FLAG); \
  } \
  else \
  { \
    K.W=R->A-Rg-(~R->P&C_FLAG); \
    R->P&=~(N_FLAG|V_FLAG|Z_FLAG|C_FLAG); \
    R->P|=((R->A^Rg)&(R->A^K.B.l)&0x80? V_FLAG:0)| \
          (K.B.h? 0:C_FLAG)|ZNTable[K.B.l]; \
    R->A=K.B.l; \
  }

#endif /* NO_DECIMAL */

#define M_CMP(Rg1,Rg2) \
  K.W=Rg1-Rg2; \
  R->P&=~(N_FLAG|Z_FLAG|C_FLAG); \
  R->P|=ZNTable[K.B.l]|(K.B.h? 0:C_FLAG)
#define M_BIT(Rg) \
  R->P&=~(N_FLAG|V_FLAG|Z_FLAG); \
  R->P|=(Rg&(N_FLAG|V_FLAG))|(Rg&R->A? 0:Z_FLAG)

#define M_AND(Rg)    R->A&=Rg;M_FL(R->A)
#define M_ORA(Rg)    R->A|=Rg;M_FL(R->A)
#define M_EOR(Rg)    R->A^=Rg;M_FL(R->A)
#define M_INC(Rg)    Rg++;M_FL(Rg)
#define M_DEC(Rg)    Rg--;M_FL(Rg)

#define M_ASL(Rg)    R->P&=~C_FLAG;R->P|=Rg>>7;Rg<<=1;M_FL(Rg)
#define M_LSR(Rg)    R->P&=~C_FLAG;R->P|=Rg&C_FLAG;Rg>>=1;M_FL(Rg)
#define M_ROL(Rg)    K.B.l=(Rg<<1)|(R->P&C_FLAG); \
            R->P&=~C_FLAG;R->P|=Rg>>7;Rg=K.B.l; \
            M_FL(Rg)
#define M_ROR(Rg)    K.B.l=(Rg>>1)|(R->P<<7); \
            R->P&=~C_FLAG;R->P|=Rg&C_FLAG;Rg=K.B.l; \
            M_FL(Rg)

/** Reset6502() **********************************************/
/** This function can be used to reset the registers before **/
/** starting execution with Run6502(). It sets registers to **/
/** their initial values.                                   **/
/*************************************************************/
void Reset6502(M6502 *R)
{
  R->A=R->X=R->Y=0x00;
  R->P=Z_FLAG;
  R->S=0xFF;
  R->PC.B.l=Rd6502(0xFFFC);
  R->PC.B.h=Rd6502(0xFFFD);   
  R->ICount=R->IPeriod;
  R->IRequest=INT_NONE;
  R->AfterCLI=0;
}

/** Exec6502() ***********************************************/
/** This function will execute a single 6502 opcode. It     **/
/** will then return next PC, and current register values   **/
/** in R.                                                   **/
/*************************************************************/
word Exec6502(M6502 *R)
{
  register pair J,K;
  register byte I;

  I=Op6502(R->PC.W++);
  R->ICount-=Cycles[I];
  switch(I)
  {
#include "Codes.h"
  }

  /* We are done */
  return(R->PC.W);
}

/** Int6502() ************************************************/
/** This function will generate interrupt of a given type.  **/
/** INT_NMI will cause a non-maskable interrupt. INT_IRQ    **/
/** will cause a normal interrupt, unless I_FLAG set in R.  **/
/*************************************************************/
void Int6502(M6502 *R,byte Type)
{
  register pair J;

  if((Type==INT_NMI)||((Type==INT_IRQ)&&!(R->P&I_FLAG)))
  {
    R->ICount-=7;
    M_PUSH(R->PC.B.h);
    M_PUSH(R->PC.B.l);
    M_PUSH(R->P & ~(B_FLAG|R_FLAG));
    R->P&=~D_FLAG;
    if(R->IAutoReset&&(Type==R->IRequest)) R->IRequest=INT_NONE;
    if(Type==INT_NMI) J.W=0xFFFA; else { R->P|=I_FLAG;J.W=0xFFFE; }
    R->PC.B.l=Rd6502(J.W++);
    R->PC.B.h=Rd6502(J.W);
  }
}

#ifdef TRACE_EXECUTION

enum Addressing_Modes
{
   Ac = 0, Il, Im, Ab, Zp, Zx, Zy, Ax, Ay, Rl, Ix, Iy, In, No
};

static char *mnCAP[] = 
{
   "ADC", "AND", "ASL", "BCC", "BCS", "BEQ", "BIT", "BMI", 
   "BNE", "BPL", "BRK", "BVC", "BVS", "CLC", "CLD", "CLI", 
   "CLV", "CMP", "CPX", "CPY", "DEC", "DEX", "DEY", "INX", 
   "INY", "EOR", "INC", "JMP", "JSR", "LDA", "NOP", "LDX", 
   "LDY", "LSR", "ORA", "PHA", "PHP", "PLA", "PLP", "ROL", 
   "ROR", "RTI", "RTS", "SBC", "STA", "STX", "STY", "SEC", 
   "SED", "SEI", "TAX", "TAY", "TXA", "TYA", "TSX", "TXS" 
};

#define DAsm DAsmCAP

static byte ad[512] = 
{
   10, Il, 34, Ix, No, No, No, No, No, No, 34, Zp, 2, Zp, No, No, 
   36, Il, 34, Im, 2, Ac, No, No, No, No, 34, Ab, 2, Ab, No, No, 
   9, Rl, 34, Iy, No, No, No, No, No, No, 34, Zx, 2, Zx, No, No, 
   13, Il, 34, Ay, No, No, No, No, No, No, 34, Ax, 2, Ax, No, No, 
   28, Ab, 1, Ix, No, No, No, No, 6, Zp, 1, Zp, 39, Zp, No, No, 
   38, Il, 1, Im, 39, Ac, No, No, 6, Ab, 1, Ab, 39, Ab, No, No, 
   7, Rl, 1, Iy, No, No, No, No, No, No, 1, Zx, 39, Zx, No, No, 
   47, Il, 1, Ay, No, No, No, No, No, No, 1, Ax, 39, Ax, No, No, 
   41, Il, 25, Ix, No, No, No, No, No, No, 25, Zp, 33, Zp, No, No, 
   35, Il, 25, Im, 33, Ac, No, No, 27, Ab, 25, Ab, 33, Ab, No, No, 
   11, Rl, 25, Iy, No, No, No, No, No, No, 25, Zx, 33, Zx, No, No, 
   15, Il, 25, Ay, No, No, No, No, No, No, 25, Ax, 33, Ax, No, No, 
   42, Il, 0, Ix, No, No, No, No, No, No, 0, Zp, 40, Zp, No, No, 
   37, Il, 0, Im, 40, Ac, No, No, 27, In, 0, Ab, 40, Ab, No, No, 
   12, Rl, 0, Iy, No, No, No, No, No, No, 0, Zx, 40, Zx, No, No, 
   49, Il, 0, Ay, No, No, No, No, No, No, 0, Ax, 40, Ax, No, No, 
   No, No, 44, Ix, No, No, No, No, 46, Zp, 44, Zp, 45, Zp, No, No, 
   22, Il, No, No, 52, Il, No, No, 46, Ab, 44, Ab, 45, Ab, No, No, 
   3, Rl, 44, Iy, No, No, No, No, 46, Zx, 44, Zx, 45, Zy, No, No, 
   53, Il, 44, Ay, 55, Il, No, No, No, No, 44, Ax, No, No, No, No, 
   32, Im, 29, Ix, 31, Im, No, No, 32, Zp, 29, Zp, 31, Zp, No, No, 
   51, Il, 29, Im, 50, Il, No, No, 32, Ab, 29, Ab, 31, Ab, No, No, 
   4, Rl, 29, Iy, No, No, No, No, 32, Zx, 29, Zx, 31, Zy, No, No, 
   16, Il, 29, Ay, 54, Il, No, No, 32, Ax, 29, Ax, 31, Ay, No, No, 
   19, Im, 17, Ix, No, No, No, No, 19, Zp, 17, Zp, 20, Zp, No, No, 
   24, Il, 17, Im, 21, Il, No, No, 19, Ab, 17, Ab, 20, Ab, No, No, 
   8, Rl, 17, Iy, No, No, No, No, No, No, 17, Zx, 20, Zx, No, No, 
   14, Il, 17, Ay, No, No, No, No, No, No, 17, Ax, 20, Ax, No, No, 
   18, Im, 43, Ix, No, No, No, No, 18, Zp, 43, Zp, 26, Zp, No, No, 
   23, Il, 43, Im, 30, Il, No, No, 18, Ab, 43, Ab, 26, Ab, No, No, 
   5, Rl, 43, Iy, No, No, No, No, No, No, 43, Zx, 26, Zx, No, No, 
   48, Il, 43, Ay, No, No, No, No, No, No, 43, Ax, 26, Ax, No, No 
};

#define RDWORD(A) (Rd6502(A+1)*256+Rd6502(A))

/** DAsm() ****************************************************/ 
/** This function will disassemble a single command and      **/ 
/** return the number of bytes disassembled.                 **/ 
/**************************************************************/ 
int DAsmCAP(char *S, word A) 
{
   
   byte J;
   
   word B, OP, TO;
   
   
   B = A;
   OP = Rd6502(B++) * 2;
   
   
   switch (ad[OP + 1])
   
   {
         
      case Ac:
         sprintf(S, "%s A", mnCAP[ad[OP]]);
         break;
         
      case Il:
         sprintf(S, "%s", mnCAP[ad[OP]]);
         break;
         
         
      case Rl:
         J = Rd6502(B++);
         TO = A + 2 + ((J < 0x80) ? J : (J - 256));
         
         sprintf(S, "%s $%04x", mnCAP[ad[OP]], TO);
         break;
         
         
      case Im:
         sprintf(S, "%s #$%02x", mnCAP[ad[OP]], Rd6502(B++));
         break;
         
      case Zp:
         sprintf(S, "%s $%02x", mnCAP[ad[OP]], Rd6502(B++));
         break;
         
      case Zx:
         sprintf(S, "%s $%02x,X", mnCAP[ad[OP]], Rd6502(B++));
         break;
         
      case Zy:
         sprintf(S, "%s $%02x,Y", mnCAP[ad[OP]], Rd6502(B++));
         break;
         
      case Ix:
         sprintf(S, "%s ($%02x,X)", mnCAP[ad[OP]], Rd6502(B++));
         break;
         
      case Iy:
         sprintf(S, "%s ($%02x),Y", mnCAP[ad[OP]], Rd6502(B++));
         break;
         
         
      case Ab:
         sprintf(S, "%s $%04x", mnCAP[ad[OP]], RDWORD(B));
         B += 2;
         break;
         
      case Ax:
         sprintf(S, "%s $%04x,X", mnCAP[ad[OP]], RDWORD(B));
         B += 2;
         break;
         
      case Ay:
         sprintf(S, "%s $%04x,Y", mnCAP[ad[OP]], RDWORD(B));
         B += 2;
         break;
         
      case In:
         sprintf(S, "%s ($%04x)", mnCAP[ad[OP]], RDWORD(B));
         B += 2;
         break;
         
         
      default:
         sprintf(S, ".db $%02x; <Invalid OPcode>", OP / 2);
         
   } 
   return (B - A);
   
} 

extern unsigned short ScanLine;

#endif

/** Run6502() ************************************************/
/** This function will run 6502 code until Loop6502() call  **/
/** returns INT_QUIT. It will return the PC at which        **/
/** emulation stopped, and current register values in R.    **/
/*************************************************************/
word Run6502(M6502 *R)
{
  register pair J,K;
  register byte I;
   byte nb_of_cycle;
  for(;;)
  {
#ifdef DEBUG
    /* Turn tracing on when reached trap address */
    if(R->PC.W==R->Trap) R->Trace=1;
    /* Call single-step debugger, exit if requested */
    if(R->Trace)
      if(!Debug6502(R)) return(R->PC.W);
#endif
     
#ifdef TRACE_EXECUTION
     while(1)
     {

     static char FA[8] = "NV.BDIZC";    
     char S[128];
     byte F;
     int J, I;
     
     DAsm(S, R->PC.W);
     
     printf 
     (
      "AT PC: [%02x - %s]\n", 
      Rd6502(R->PC.W), S
     );
        break;
     }
     
#endif
     I=Op6502(R->PC.W++);
     nb_of_cycle = Cycles[I];
//#ifdef DEBUG
//    pushop(I);
//#endif
    icount++;
    switch(I)
    {
#include "Codes.h"
    }
#ifdef TRACE_EXECUTION
     while(1)
     {
           static char FA[8] = "NV.BDIZC";    
           char S[128];
           byte F;
           int J, I;
           
     printf 
     (
      "A:%02x X:%02x Y:%02x S:%04x, PC:%04x Flags:[", 
      R->A, R->X, R->Y, R->S + 0x0100, R->PC.W 
      );
     
     
     for (J = 0, F = R->P; J < 8; J++, F <<= 1)
        
        printf("%c", F & 0x80 ? FA[J] : '.');
     
        printf("], Stack[%02x, %02x, %02x], %03d, %03d\n",
               Rd6502(0x0100 + (byte) (R->S + 1)),
               Rd6502(0x0100 + (byte) (R->S + 2)),
               Rd6502(0x0100 + (byte) (R->S + 3)),
		     R->ICount,
			ScanLine
	        );
   
        break;
     }
#endif
     R->ICount-= nb_of_cycle;
    /* If cycle counter expired... */
    if(R->ICount<=0)
    {
      /* If we have come after CLI, get INT_? from IRequest */
      /* Otherwise, get it from the loop handler            */
      if(R->AfterCLI)
      {
        I=R->IRequest;            /* Get pending interrupt     */
        R->ICount+=R->IBackup-1;  /* Restore the ICount        */
        R->AfterCLI=0;            /* Done with AfterCLI state  */
      }
      else
      {
        I=Loop6502(R);            /* Call the periodic handler */
        R->ICount+=R->IPeriod;    /* Reset the cycle counter   */
        if(!I) I=R->IRequest;     /* Realize pending interrupt */
      }

      if(I==INT_QUIT) return(R->PC.W); /* Exit if INT_QUIT     */
      if(I) Int6502(R,I);              /* Interrupt if needed  */ 
    }
  }

  /* Execution stopped */
  return(R->PC.W);
}
