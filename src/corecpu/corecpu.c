/**
 *  CoreCPU - The Quick6502 Project
 *  corecpu.c
 *
 *  Created by Manoel Trapier on 24/02/08
 *  Copyright 2008 986 Corp. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

/* Depending on the OS, one of these provide the malloc function */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <os_dependent.h>

/*******************************************************************************
 /!\ WARNING this debug tool slow down a lot the emulator!                   /!\
 /!\ Use it only if you really need it !                                     /!\
 *******************************************************************************/
//#define TRACE_INSTRUCTIONS

#ifdef TRACE_INSTRUCTIONS
#define TRACEi(trace) do { console_printf(Console_Debug, "$%04X - ", cpu->reg_PC - 1); console_printf_d trace ; console_printf(Console_Debug, "\n"); } while(0)
#define TRACEiE(trace) do { console_printf(Console_Debug, "$%04X - ", cpu->reg_PC - 1); console_printf_d trace ; console_printf(Console_Debug, "\n"); } while(0)
#else
#define TRACEi(trace) { }
//#define TRACEiE(trace) { }
#define TRACEiE(trace) do { console_printf(Console_Debug, "$%04X - ", cpu->reg_PC - 1); console_printf_d trace ; console_printf(Console_Debug, "\n"); } while(0)
#endif

#define _INTERNAL_QUICK6502_CORECPU_
#include "corecpu.h"


/*** Instructions useful macros ***/
#define INSTRUCTION(s) static inline void I_##s (quick6502_cpu *cpu)

#define NZ_FLAG_UPDATE(value) cpu->reg_P = (cpu->reg_P & ~(Q6502_N_FLAG | Q6502_Z_FLAG)) | \
                                           (value & 0x80) | ((value)?0:Q6502_Z_FLAG)

#define CROSS_CYCLE_UPDATE(value) if ((value) & 0x0F00) cpu->page_crossed = 1

#define MEMORY_READ_ZP() cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC++))

#define MEMORY_READ_IX() cpu->memory_read( (cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC  ) + cpu->reg_X    ) & 0xFF) |\
                                           (cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC++) + cpu->reg_X + 1) << 8) )
#define MEMORY_READ_IY() cpu->memory_read( ( cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC  )    ) |\
                                            (cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC++) + 1) << 8) ) + cpu->reg_Y  )

#define MEMORY_READ_ZX() cpu->memory_page0_read( (cpu->memory_opcode_read(cpu->reg_PC++) + cpu->reg_X) )
#define MEMORY_READ_ZY() cpu->memory_page0_read( (cpu->memory_opcode_read(cpu->reg_PC++) + cpu->reg_Y) )

#define MEMORY_READ_AB() cpu->memory_read( ((cpu->memory_opcode_read(cpu->reg_PC++)     ) |\
                                            (cpu->memory_opcode_read(cpu->reg_PC++) << 8) ))

#define MEMORY_READ_AX() cpu->memory_read( ((op1     ) |\
                                            (op2 << 8) ) + cpu->reg_X)
#define MEMORY_READ_AY() cpu->memory_read( ((op1     ) |\
                                            (op2 << 8) ) + cpu->reg_Y )


#define MEMORY_WRITE_ZP(val) cpu->memory_page0_write(cpu->memory_opcode_read(cpu->reg_PC++), val)

#define MEMORY_WRITE_IX(val) cpu->memory_write( (cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC  ) + cpu->reg_X    ) & 0xFF) |\
                                                 (cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC++) + cpu->reg_X + 1) << 8)   , val)
#define MEMORY_WRITE_IY(val) cpu->memory_write( ( cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC  )    ) |\
                                                 (cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC++) + 1) << 8) ) + cpu->reg_Y , val)

#define MEMORY_WRITE_ZX(val) cpu->memory_page0_write( (cpu->memory_opcode_read(cpu->reg_PC++) + cpu->reg_X), val)
#define MEMORY_WRITE_ZY(val) cpu->memory_page0_write( (cpu->memory_opcode_read(cpu->reg_PC++) + cpu->reg_Y), val)

#define MEMORY_WRITE_AB(val) cpu->memory_write( ((cpu->memory_opcode_read(cpu->reg_PC++)     ) |\
                                                 (cpu->memory_opcode_read(cpu->reg_PC++) << 8) ), val)

#define MEMORY_WRITE_AX(val) cpu->memory_write( ((cpu->memory_opcode_read(cpu->reg_PC++)     ) |\
                                                 (cpu->memory_opcode_read(cpu->reg_PC++) << 8) ) + cpu->reg_X, val)
#define MEMORY_WRITE_AY(val) cpu->memory_write( ((cpu->memory_opcode_read(cpu->reg_PC++)     ) |\
                                                 (cpu->memory_opcode_read(cpu->reg_PC++) << 8) ) + cpu->reg_Y, val)


#define PUSH_S(value) cpu->memory_stack_write(0x100 | (cpu->reg_S--), value)
#define POP_S()      (cpu->memory_stack_read (0x100 | (++cpu->reg_S)       ))

#ifdef NO_DECIMAL

#define ADC_OPERATION(read) do {\
   unsigned short tmp = 0; unsigned char v = read; \
   tmp = cpu->reg_A + v + (cpu->reg_P & Q6502_C_FLAG); \
   cpu->reg_P = (cpu->reg_P & ~(Q6502_C_FLAG | Q6502_N_FLAG | Q6502_Z_FLAG | Q6502_V_FLAG)) | \
          (tmp & 0x80) | ((tmp&0xFF)?0:Q6502_Z_FLAG) | \
         ((tmp & 0xFF00)?Q6502_C_FLAG:0) | \
          ( ( ~(cpu->reg_A^v)&(cpu->reg_A^tmp) )&0x80?Q6502_V_FLAG:0 ); \
   cpu->reg_A = tmp & 0xFF; \
} while(0)

#define SBC_OPERATION(read) do {\
   unsigned short tmp = 0; unsigned char v = read; \
   tmp = cpu->reg_A - v - (~cpu->reg_P & Q6502_C_FLAG); \
   cpu->reg_P = (cpu->reg_P & ~(Q6502_C_FLAG | Q6502_N_FLAG | Q6502_Z_FLAG | Q6502_V_FLAG)) | \
          (tmp & Q6502_N_FLAG) | ((tmp&0xFF)?0:Q6502_Z_FLAG) | \
         ((tmp & 0xFF00)?0:Q6502_C_FLAG) | \
          ( ( (cpu->reg_A^v)&(cpu->reg_A^tmp) )&0x80?Q6502_V_FLAG:0 ); \
   cpu->reg_A = tmp & 0xFF; \
} while(0)

#else
#error Quick6502 doesn't actually support DECIMAL mode
#endif


#define AND_OPERATION(read) cpu->reg_A &= read; NZ_FLAG_UPDATE(cpu->reg_A)

/* CMP is like SBC but without storing the result value */
#define CMP_OPERATION(register, read) do { \
   unsigned short tmp = 0; \
   tmp = register - read; \
   cpu->reg_P = (cpu->reg_P & ~(Q6502_C_FLAG | Q6502_N_FLAG | Q6502_Z_FLAG)) | \
          (tmp & Q6502_N_FLAG) | ((tmp&0xFF)?0:Q6502_Z_FLAG) | \
          ((tmp & 0xFF00)?0:Q6502_C_FLAG); \
} while(0)

#define EOR_OPERATION(read) cpu->reg_A ^= read; NZ_FLAG_UPDATE(cpu->reg_A)
#define ORA_OPERATION(read) cpu->reg_A |= read; NZ_FLAG_UPDATE(cpu->reg_A)

#define BIT_OPERATION(read) do { \
   byte tmp = read; \
   cpu->reg_P = (cpu->reg_P & ~(Q6502_V_FLAG | Q6502_N_FLAG | Q6502_Z_FLAG)) | \
          (tmp & Q6502_N_FLAG) | (tmp & Q6502_V_FLAG) | \
          ((tmp & cpu->reg_A)?0:Q6502_Z_FLAG); \
} while(0)

#define ASL_OPERATION(val) cpu->reg_P = (cpu->reg_P & ~(Q6502_C_FLAG | Q6502_N_FLAG | Q6502_Z_FLAG)) | \
                          ((val&0x40)?Q6502_N_FLAG:0) | \
                          ((val&0x80)?Q6502_C_FLAG:0) | \
                          ((val&0x7F)?0:Q6502_Z_FLAG); \
                 val = val << 1

#define LSR_OPERATION(val) cpu->reg_P = (cpu->reg_P & ~(Q6502_C_FLAG | Q6502_N_FLAG | Q6502_Z_FLAG)) | \
                          (val & Q6502_C_FLAG) | \
                          ((val&0xFE)?0:Q6502_Z_FLAG); \
                 val = val >> 1

#define ROR_OPERATION(val) do {\
   unsigned short tmp = val | (cpu->reg_P & Q6502_C_FLAG) << 8; \
   cpu->reg_P = (cpu->reg_P & ~(Q6502_C_FLAG | Q6502_N_FLAG | Q6502_Z_FLAG)) | \
          ( tmp&Q6502_C_FLAG) |         /* Set the C flag */ \
          ((tmp&0x100) >> 1) |          /* Set the N flag */ \
          ((tmp&0x1FE)?0:Q6502_Z_FLAG); /* 0x1FE will be the new 8bit value */ \
   val = (tmp>>1) & 0xFF; \
} while(0)

#define ROL_OPERATION(val) do {\
unsigned short tmp = (val << 1) | (cpu->reg_P & Q6502_C_FLAG); \
   cpu->reg_P = (cpu->reg_P & ~(Q6502_C_FLAG | Q6502_N_FLAG | Q6502_Z_FLAG)) | \
         ((tmp&0x100)?Q6502_C_FLAG:0) | /* Set the C flag */ \
         ((tmp&0x80)) |                 /* Set the N flag */ \
         ((tmp&0xFF)?0:Q6502_Z_FLAG);   /* 0x1FE will be the new 8bit value */ \
   val = tmp &   0xFF; \
} while(0)


/** Function used for execution of instruction */
static inline int quick6502_exec_one(quick6502_cpu *cpu);

/**
 * Initialise the CPU
 *
 * Inputs:
 *
 * - CPU Init structure:
 *  - Memory Read function pointer
 *  - Memory Write function pointer
 *  - Fast memory read function pointer (for opcodes read)
 *  - Fast page 0 function Read/Write
 *  - Fast page 1 function Read/Write
 *
 * Output:
 *
 * (void *): An opaque pointer to the internal structure of the CPU.
 *           NULL if an error occured !
 */
quick6502_cpu *quick6502_init(quick6502_cpuconfig *config)
{
   quick6502_cpu *cpu;

   /* Alloc structure */
   cpu = (quick6502_cpu *) malloc (sizeof (quick6502_cpu));
   if (!cpu)
      return NULL;

   /* Initialise other variables */
   cpu->running = 0; /* CPU is currently NOT running */

   cpu->cycle_done = 0;
   cpu->int_pending = 0;
   
   cpu->page_crossed = 0;
   
   /* Initialise registers */
   cpu->reg_A = 0;
   cpu->reg_X = 0;
   cpu->reg_Y = 0;
   cpu->reg_S = 0xFF;
   
   cpu->reg_P = Q6502_D_FLAG | Q6502_I_FLAG;
   
   if (config->memory_read != NULL)
      cpu->memory_read                 = config->memory_read;
   else
      goto init_error;

   if (config->memory_write != NULL)
      cpu->memory_write                = config->memory_write;
   else
      goto init_error;

   if (config->memory_opcode_read != NULL)
      cpu->memory_opcode_read          = config->memory_opcode_read;
   else
      cpu->memory_opcode_read          = config->memory_read;


   if (config->memory_page0_read != NULL)
      cpu->memory_page0_read           = config->memory_page0_read;
   else
      cpu->memory_page0_read           = config->memory_read;

   if (config->memory_page0_write != NULL)
      cpu->memory_page0_write          = config->memory_page0_write;
   else
      cpu->memory_page0_write          = config->memory_write;

   if (config->memory_stack_read != NULL)
      cpu->memory_stack_read           = config->memory_stack_read;
   else
      cpu->memory_stack_read           = config->memory_read;

   if (config->memory_stack_write != NULL)
      cpu->memory_stack_write          = config->memory_stack_write;
   else
      cpu->memory_stack_write          = config->memory_write;
   
   return cpu;

init_error:
   if (cpu)
      free (cpu);

   return NULL;
}


/** Reset the CPU (must be done after init) */
void quick6502_reset(quick6502_cpu *cpu)
{
   /* Initialise registers */
   /*cpu->reg_A = 0;
   cpu->reg_X = 0;
   cpu->reg_Y = 0;
   cpu->reg_S = 0xFF;*/

   //cpu->reg_P = Q6502_D_FLAG | Q6502_I_FLAG | 0x20 | Q6502_B_FLAG;

   /* Set the PC to the RESET vector */
   cpu->reg_PC = ( cpu->memory_read(Q6502_RESET_HIGH) << 8)
                 | cpu->memory_read(Q6502_RESET_LOW);

   cpu->exit_loop = 0;
}

/**
 * Run cpu for at least X cycles 
 *
 * Output:
 *
 * int: (Number of cycle really done) - (Number of cycle asked)
 */
int quick6502_run(quick6502_cpu *cpu, int cycles)
{
   cpu->running = !0;

   while(cpu->cycle_done < cycles)
   {
      quick6502_exec_one(cpu);
   }
   cpu->cycle_done -= cycles;

   cpu->running = 0;

   return cycles + cpu->cycle_done;
}

/** Loop CPU until explicit quit */
void quick6502_loop(quick6502_cpu *cpu)
{
   cpu->running = !0;
   while(cpu->exit_loop)
   {
      quick6502_exec_one(cpu);
   }
   cpu->running = 0;
}

/** Run CPU for one instruction */
void quick6502_exec(quick6502_cpu *cpu)
{
   cpu->running = !0;
   quick6502_exec_one(cpu);
   cpu->running = 0;
}

/** Send IRQ/NMI/EXITLOOP signal to CPU */
void quick6502_int(quick6502_cpu *cpu, quick6502_signal signal)
{
   switch(signal)
   {
    default:
       break;

    case Q6502_IRQ_SIGNAL:
       if (! (cpu->reg_P & Q6502_I_FLAG) )
       {
          TRACEi(("IRQ Triggered !"));
          PUSH_S((cpu->reg_PC >> 8) & 0xFF   );
          PUSH_S((cpu->reg_PC     ) & 0xFF   );
          PUSH_S( cpu->reg_P & ~Q6502_B_FLAG );
          cpu->reg_P = cpu->reg_P | Q6502_I_FLAG;

          cpu->reg_PC = (cpu->memory_read(Q6502_IRQ_LOW)) | (cpu->memory_read(Q6502_IRQ_HIGH)<<8);
          
          cpu->cycle_done += 7;
       }
       else
          cpu->int_pending = 1;

       break;

    case Q6502_NMI_SIGNAL:
       TRACEi(("NMI Triggered !"));
       PUSH_S((cpu->reg_PC >> 8) & 0xFF   );
       PUSH_S((cpu->reg_PC     ) & 0xFF   );
       PUSH_S( cpu->reg_P                 );
       cpu->reg_P = cpu->reg_P | Q6502_I_FLAG & ~Q6502_B_FLAG;

       cpu->reg_PC = (cpu->memory_read(Q6502_NMI_LOW)) | (cpu->memory_read(Q6502_NMI_HIGH)<<8);
       
       cpu->cycle_done += 7;
       break;

    case Q6502_STOPLOOP_SIGNAL:
       cpu->exit_loop = 1;
       break;
   }
}

/** Dump CPU State to the given file */
void quick6502_dump(quick6502_cpu *cpu, FILE * fp)
{
   short i;
   char instr[20];
   /* Display registers */
   fprintf(fp, 
       "Quick6502: PC:$%04X A:$%02X X:$%02X Y:$%02X S:$%02X P:$%02X P:[%c%c%c%c%c%c%c%c]\n",
       cpu->reg_PC, cpu->reg_A, cpu->reg_X, cpu->reg_Y, cpu->reg_S, cpu->reg_P,
       cpu->reg_P&Q6502_N_FLAG ? 'N':'.',
       cpu->reg_P&Q6502_V_FLAG ? 'V':'.',
                                     '.', /* No real flag here */
       cpu->reg_P&Q6502_B_FLAG ? 'B':'.',
       cpu->reg_P&Q6502_D_FLAG ? 'D':'.',
       cpu->reg_P&Q6502_I_FLAG ? 'I':'.',
       cpu->reg_P&Q6502_Z_FLAG ? 'Z':'.',
       cpu->reg_P&Q6502_C_FLAG ? 'C':'.'
         );

   /* Display stack */
   fprintf(fp, "Quick6502: Stack: [ ");
   for (i = cpu->reg_S+1; i < 0x100; i++)
   {
    fprintf(fp, "$%02X ", cpu->memory_opcode_read(0x100 | i));
   }
   fprintf(fp, "] Run:%c Cycle:%ld\n", cpu->running?'Y':'N', cpu->cycle_done);

   quick6502_getinstruction(cpu, cpu->reg_PC, instr);
   fprintf(fp, "Quick6502: Instruction at PC: %s\n", instr);
}

/** Get current instruction name at specified address and put it into buffer */
void quick6502_getinstruction(quick6502_cpu *cpu, unsigned short addr, char *buffer)
{
   buffer[0] = 0;
}

/**
 * Free the CPU 
 *
 * This function will free the CPU only if it's not currently used, it will
 * return !0 if everything goes well and 0 if the free is impossible
 */
int quick6502_free(quick6502_cpu *cpu)
{

   return 0;
}

/*******************************************************************************
 ***                          Here start real CPU logic                      ***
 *******************************************************************************/
static byte CycleTable[256] =
{
/*        00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F */
/* 00 */   7, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
/* 10 */   2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
/* 20 */   6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
/* 30 */   2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
/* 40 */   6, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
/* 50 */   2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
/* 60 */   6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
/* 70 */   2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
/* 80 */   2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
/* 90 */   2, 6, 0, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
/* A0 */   2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
/* B0 */   2, 5, 0, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
/* C0 */   2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
/* D0 */   2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
/* E0 */   2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
/* F0 */   2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7
/*        00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F */
};

typedef void (*InstructionFunction)(quick6502_cpu *cpu);

/*******************************************************************************
 *    Instruction implementations 
 *******************************************************************************/

/**** Other instructions ****/
INSTRUCTION(ILLEG)
{
   TRACEiE(("Illegal instruction $%02X", cpu->memory_opcode_read(cpu->reg_PC-1)));
   //exit(-1);
}

/** 58 : CLI - CLear Interrupt **/
INSTRUCTION(CLIiM)
{
   TRACEi(("CLC"));
   cpu->reg_P &= ~Q6502_I_FLAG;
}
/** 78 : SEI - SEt Interrupt **/
INSTRUCTION(SEIiM)
{
   TRACEi(("SEI"));
   cpu->reg_P |= Q6502_I_FLAG;
}

/** 18 : CLC - CLear Carry **/
INSTRUCTION(CLCiM)
{
   TRACEi(("CLC"));
   cpu->reg_P &= ~Q6502_C_FLAG;
}
/** 38 : SEC - SEt Carry **/
INSTRUCTION(SECiM)
{
   TRACEi(("SEC"));
   cpu->reg_P |= Q6502_C_FLAG;
}

/** D8 : CLD - CLear Decimal **/
INSTRUCTION(CLDiM)
{
   TRACEi(("CLD"));
   cpu->reg_P &= ~Q6502_D_FLAG;
}
/** F8 : SED - SEt Decimal **/
INSTRUCTION(SEDiM)
{
   TRACEi(("SED"));
   cpu->reg_P |= Q6502_D_FLAG;
}
/** B8 : CLV - CLear oVerflo **/
INSTRUCTION(CLViM)
{
   TRACEi(("CLV"));
   cpu->reg_P &= ~Q6502_V_FLAG;
}

/** EA : NOP - NO oPeration **/
INSTRUCTION(NOPiM)
{
   TRACEi(("NOP"));
}

/**** Load/Store functions ****/

/** A9 : LDA #$xx - LoaD an immediate value into A */
INSTRUCTION(LDAiM)
{
   TRACEi(("LDA #$%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_A = cpu->memory_opcode_read(cpu->reg_PC++);
   NZ_FLAG_UPDATE(cpu->reg_A);
}

/** A2 : LDX #$xx - LoaD an immediate value into X */
INSTRUCTION(LDXiM)
{
   TRACEi(("LDX #$%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_X = cpu->memory_opcode_read(cpu->reg_PC++);
   NZ_FLAG_UPDATE(cpu->reg_X);
}

/** A0 : LDY #$xx - LoaD an immediate value into Y */
INSTRUCTION(LDYiM)
{
   TRACEi(("LDY #$%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_Y = cpu->memory_opcode_read(cpu->reg_PC++);
   NZ_FLAG_UPDATE(cpu->reg_Y);
}

/** A5: LDA $xx - LoaD to A from zero page **/
INSTRUCTION(LDAzP)
{
   TRACEi(("LDA $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_A = MEMORY_READ_ZP();
   NZ_FLAG_UPDATE(cpu->reg_A);
}

/** B5: LDA $xx,X - LoaD to A **/
INSTRUCTION(LDAzX)
{
   TRACEi(("LDA $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_A = MEMORY_READ_ZX();
   NZ_FLAG_UPDATE(cpu->reg_A);
}

/** A1: LDA ($xx,X) - LoaD to A **/
INSTRUCTION(LDAiX)
{
   TRACEi(("LDA ($%02X,X)", cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_A = MEMORY_READ_IX();
   NZ_FLAG_UPDATE(cpu->reg_A);
}

/** B1: LDA ($xx),Y - LoaD to A **/
INSTRUCTION(LDAiY)
{
   TRACEi(("LDA ($%02X),Y", cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_A = MEMORY_READ_IY();
   NZ_FLAG_UPDATE(cpu->reg_A);
   CROSS_CYCLE_UPDATE(cpu->memory_opcode_read(cpu->reg_PC-1) + cpu->reg_Y);
}

/** AD: LDA $xxxx - LoaD to A **/
INSTRUCTION(LDAaB)
{
   TRACEi(("LDA $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_A = MEMORY_READ_AB();
   NZ_FLAG_UPDATE(cpu->reg_A);
}

/** DD: LDA $xxxx,X - LoaD to A **/
INSTRUCTION(LDAaX)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   
   TRACEi(("LDA $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_A = MEMORY_READ_AX();
   NZ_FLAG_UPDATE(cpu->reg_A);
   CROSS_CYCLE_UPDATE(op1 + cpu->reg_X);
}

/** D9: LDA $xxxx,Y - LoaD to A **/
INSTRUCTION(LDAaY)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   cpu->reg_A = MEMORY_READ_AY();
   NZ_FLAG_UPDATE(cpu->reg_A);
   CROSS_CYCLE_UPDATE(op1 + cpu->reg_Y);
}

/** 85: STA $xx - STore A to zero page **/
INSTRUCTION(STAzP)
{
   TRACEi(("STA $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->memory_page0_write(cpu->memory_opcode_read(cpu->reg_PC++), cpu->reg_A);
}

/** 95: STA $xx,X - STore A **/
INSTRUCTION(STAzX)
{
   TRACEi(("STA $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
   MEMORY_WRITE_ZX(cpu->reg_A);
}

/** 81: STA ($xx,X) - STore A **/
INSTRUCTION(STAiX)
{
   TRACEi(("STA ($%02X,X)", cpu->memory_opcode_read(cpu->reg_PC)));
   MEMORY_WRITE_IX(cpu->reg_A);
}

/** 91: STA ($xx),Y - STore A **/
INSTRUCTION(STAiY)
{
   TRACEi(("STA ($%02X),Y", cpu->memory_opcode_read(cpu->reg_PC)));
   MEMORY_WRITE_IY(cpu->reg_A);
}

/** 8D: STA $xxxx - STore A **/
INSTRUCTION(STAaB)
{
   TRACEi(("STA $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   MEMORY_WRITE_AB(cpu->reg_A);
}

/** 9D: STA $xxxx,X - STore A **/
INSTRUCTION(STAaX)
{
   TRACEi(("STA $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   MEMORY_WRITE_AX(cpu->reg_A);
}

/** 99: STA $xxxx,Y - STore A **/
INSTRUCTION(STAaY)
{
   TRACEi(("STA $%02X%02X,Y", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   MEMORY_WRITE_AY(cpu->reg_A);
}

/** A6: LDX $xx - LoaD to X from zero page **/
INSTRUCTION(LDXzP)
{
   TRACEi(("LDX $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_X = MEMORY_READ_ZP();
   NZ_FLAG_UPDATE(cpu->reg_X);
}

/** B6: LDX $xx,Y - LoaD to X **/
INSTRUCTION(LDXzY)
{
   TRACEi(("LDX $%02X,Y", cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_X = MEMORY_READ_ZY();
   NZ_FLAG_UPDATE(cpu->reg_X);
}

/** AE: LDX $xxxx - LoaD to X **/
INSTRUCTION(LDXaB)
{
   TRACEi(("LDX $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_X = MEMORY_READ_AB();
   NZ_FLAG_UPDATE(cpu->reg_X);
}

/** BE: LDX $xxxx,Y - LoaD to X **/
INSTRUCTION(LDXaY)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   TRACEi(("LDX $%02X%02X,Y", op2, op1));
   cpu->reg_X = MEMORY_READ_AY();
   NZ_FLAG_UPDATE(cpu->reg_X);
   CROSS_CYCLE_UPDATE(op1 + cpu->reg_Y);
}

/** B6: STX $xx - STore X to zero page **/
INSTRUCTION(STXzP)
{
   TRACEi(("STX $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->memory_page0_write(cpu->memory_opcode_read(cpu->reg_PC++), cpu->reg_X);
}

/** 96: STX $xx,Y - STore X **/
INSTRUCTION(STXzY)
{
   TRACEi(("STX $%02X,Y", cpu->memory_opcode_read(cpu->reg_PC)));
   MEMORY_WRITE_ZY(cpu->reg_X);
}

/** 8E: STX $xxxx - STore X **/
INSTRUCTION(STXaB)
{
   TRACEi(("STX $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   MEMORY_WRITE_AB(cpu->reg_X);
}

/** A4: LDY $xx - LoaD to Y from zero page **/
INSTRUCTION(LDYzP)
{
   TRACEi(("LDY $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_Y = MEMORY_READ_ZP();
   NZ_FLAG_UPDATE(cpu->reg_Y);
}

/** B4: LDY $xx,X - LoaD to Y **/
INSTRUCTION(LDYzX)
{
   TRACEi(("LDY $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_Y = MEMORY_READ_ZX();
   NZ_FLAG_UPDATE(cpu->reg_Y);
}

/** AC: LDY $xxxx - LoaD to Y **/
INSTRUCTION(LDYaB)
{
   TRACEi(("LDY $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_Y = MEMORY_READ_AB();
   NZ_FLAG_UPDATE(cpu->reg_Y);
}

/** BC: LDY $xxxx,X - LoaD to Y **/
INSTRUCTION(LDYaX)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   
   TRACEi(("LDY $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_Y = MEMORY_READ_AX();
   NZ_FLAG_UPDATE(cpu->reg_Y);
   
   CROSS_CYCLE_UPDATE(op1 + cpu->reg_X);
}

/** 84: STY $xx - STore Y to zero page **/
INSTRUCTION(STYzP)
{
   TRACEi(("STY $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->memory_page0_write(cpu->memory_opcode_read(cpu->reg_PC++), cpu->reg_Y);
}

/** 94: STY $xx,X - STore Y **/
INSTRUCTION(STYzX)
{
   TRACEi(("STY $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
   MEMORY_WRITE_ZX(cpu->reg_Y);
}

/** 8C: STY $xxxx - STore Y **/
INSTRUCTION(STYaB)
{
   TRACEi(("STY $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   MEMORY_WRITE_AB(cpu->reg_Y);
}

/**** Register functions ****/

/** AA : TAX - Transfer A to X **/
INSTRUCTION(TAXiM)
{
   TRACEi(("TAX"));
   cpu->reg_X = cpu->reg_A;
   NZ_FLAG_UPDATE(cpu->reg_X);
}

/** 8A : TXA - Transfer X to A **/
INSTRUCTION(TXAiM)
{
   TRACEi(("TXA"));
   cpu->reg_A = cpu->reg_X;
   NZ_FLAG_UPDATE(cpu->reg_A);
}

/** A8 : TAY - Transfer A to Y **/
INSTRUCTION(TAYiM)
{
   TRACEi(("TAY"));
   cpu->reg_Y = cpu->reg_A;
   NZ_FLAG_UPDATE(cpu->reg_Y);
}

/** 98 : TYA - Transfer Y to A **/
INSTRUCTION(TYAiM)
{
   TRACEi(("TYA"));
   cpu->reg_A = cpu->reg_Y;
   NZ_FLAG_UPDATE(cpu->reg_A);
}

/* BA : TSX - Transfer S to X **/
INSTRUCTION(TSXiM)
{
   TRACEi(("TSX"));
   cpu->reg_X = cpu->reg_S;
   NZ_FLAG_UPDATE(cpu->reg_X);
}

/** 9A : TXS - Transfer X to S **/
INSTRUCTION(TXSiM)
{
   TRACEi(("TXS"));
   cpu->reg_S = cpu->reg_X;
}

/**** Simple register operation instructions ****/

/** CA : DEX - DEcrement X **/
INSTRUCTION(DEXiM)
{
   TRACEi(("DEX"));
   cpu->reg_X --;
   NZ_FLAG_UPDATE(cpu->reg_X);
}

/** 88 : DEY - DEcrement Y **/
INSTRUCTION(DEYiM)
{
   TRACEi(("DEY"));
   cpu->reg_Y --;
   NZ_FLAG_UPDATE(cpu->reg_Y);
}

/** E8 : INX - INcrement X **/
INSTRUCTION(INXiM)
{
   TRACEi(("INX"));
   cpu->reg_X ++;
   NZ_FLAG_UPDATE(cpu->reg_X);
}

/** C8 : INY - INcrement Y **/
INSTRUCTION(INYiM)
{
   TRACEi(("INY"));
   cpu->reg_Y ++;
   NZ_FLAG_UPDATE(cpu->reg_Y);
}

/**** Stack related instructions ****/

/** 48 : PHA - PusH A */
INSTRUCTION(PHAiM)
{
   TRACEi(("PHA"));
   PUSH_S(cpu->reg_A);
}

/** 68 : PLA - PuLl A */
INSTRUCTION(PLAiM)
{
   TRACEi(("PLA"));
   cpu->reg_A = POP_S();
   NZ_FLAG_UPDATE(cpu->reg_A);
}

/** 08 : PHP - PusH P */
INSTRUCTION(PHPiM)
{
   TRACEi(("PHP"));
   PUSH_S((cpu->reg_P | Q6502_R_FLAG | Q6502_B_FLAG));
}

/** 28 : PLP - PuLl P */
INSTRUCTION(PLPiM)
{
   TRACEi(("PLP"));
   cpu->reg_P = POP_S() & ~(Q6502_R_FLAG | Q6502_B_FLAG);
   if (cpu->int_pending != 0)
   {
      quick6502_int(cpu, Q6502_IRQ_SIGNAL);
   }
}

/**** Branch instructions ****/

/** 20 : JSR $xxxx - Jump to SubRoutine */
INSTRUCTION(JSRaB)
{
   TRACEi(("JSR $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_PC++;
   PUSH_S((cpu->reg_PC >> 8) & 0xFF);
   PUSH_S((cpu->reg_PC     ) & 0xFF);
   cpu->reg_PC = ((cpu->memory_opcode_read(cpu->reg_PC-1)     ) | (cpu->memory_opcode_read(cpu->reg_PC) << 8));
}

/** 60 : RTS - ReTurn from Subrutine */
INSTRUCTION(RTSiM)
{
   TRACEi(("RTS"));
   cpu->reg_PC = POP_S() | (POP_S() << 8);
   cpu->reg_PC ++;
}

/** 4C : JMP $xxxx - JuMP inconditionaly to $xxxx **/
INSTRUCTION(JMPaB)
{
   TRACEi(("JMP $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_PC = cpu->memory_opcode_read(cpu->reg_PC++) | (cpu->memory_opcode_read(cpu->reg_PC) << 8);
}

/** 6C : JMP ($xxxx) - JuMP inconditionaly to ($xxxx) **/
INSTRUCTION(JMPiD)
{
   TRACEi(("JMP ($%02X%02X)", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   cpu->reg_PC = cpu->memory_opcode_read(cpu->reg_PC) | (cpu->memory_opcode_read(cpu->reg_PC+1) << 8);
   cpu->reg_PC = cpu->memory_read(cpu->reg_PC) | 
                (cpu->memory_read((cpu->reg_PC & 0xFF00) | ((cpu->reg_PC+1) & 0x00FF)) << 8);
}

/** 00 : BRK - BReaK **/
INSTRUCTION(BRKiM)
{
   TRACEi(("BRK"));
   cpu->reg_PC++;
   PUSH_S((cpu->reg_PC >> 8) & 0xFF);
   PUSH_S((cpu->reg_PC     ) & 0xFF);
   PUSH_S( cpu->reg_P              );
   cpu->reg_P = cpu->reg_P | Q6502_I_FLAG | Q6502_B_FLAG;

   cpu->reg_PC = (cpu->memory_read(Q6502_IRQ_LOW)) | (cpu->memory_read(Q6502_IRQ_HIGH)<<8);
}

/** 40 : RTI - ReTurn from Interruption **/
INSTRUCTION(RTIiM)
{
   TRACEi(("RTI"));
   cpu->reg_P = POP_S();
   cpu->reg_PC = POP_S() | (POP_S() << 8);

   if (cpu->int_pending != 0)
   {
      quick6502_int(cpu, Q6502_IRQ_SIGNAL);
   }
}

/** 90 : BCC - Branch if Carry Clear **/
INSTRUCTION(BCCrE)
{
   TRACEi(("BCC $%04X", cpu->reg_PC + (signed char) cpu->memory_opcode_read(cpu->reg_PC) + 1));
   if (!(cpu->reg_P & Q6502_C_FLAG))
   {
      cpu->reg_PC += (signed char) cpu->memory_opcode_read(cpu->reg_PC++) + 1;
      /* Need to set timing */

      /* +1 is same page */
      cpu->cycle_done += 1;
      /* +2 is another */
   }
   else
      cpu->reg_PC ++;
}

/** B0 : BCS - Branch if Carry Set**/
INSTRUCTION(BCSrE)
{
   TRACEi(("BCS $%04X", cpu->reg_PC + (signed char) cpu->memory_opcode_read(cpu->reg_PC) + 1));
   if (cpu->reg_P & Q6502_C_FLAG)
   {
      cpu->reg_PC += (signed char) cpu->memory_opcode_read(cpu->reg_PC ++) + 1;
      /* Need to set timing */

      /* +1 is same page */
      cpu->cycle_done += 1;
      /* +2 is another */
   }
   else
      cpu->reg_PC ++;
}

/** F0 : BEQ - Branch if Equal**/
INSTRUCTION(BEQrE)
{
   TRACEi(("BEQ $%04X", cpu->reg_PC + (signed char) cpu->memory_opcode_read(cpu->reg_PC) + 1));
   if (cpu->reg_P & Q6502_Z_FLAG)
   {
      cpu->reg_PC += (signed char) cpu->memory_opcode_read(cpu->reg_PC ++) + 1;
      /* Need to set timing */

      /* +1 is same page */
      cpu->cycle_done += 1;
      /* +2 is another */
   }
   else
      cpu->reg_PC ++;
}

/** 30 : BMI - Branch if MInus**/
INSTRUCTION(BMIrE)
{
   TRACEi(("BMI $%04X", cpu->reg_PC + (signed char) cpu->memory_opcode_read(cpu->reg_PC) + 1));
   if (cpu->reg_P & Q6502_N_FLAG)
   {
      cpu->reg_PC += (signed char) cpu->memory_opcode_read(cpu->reg_PC ++) + 1;
      /* Need to set timing */

      /* +1 is same page */
      cpu->cycle_done += 1;
      /* +2 is another */
   }
   else
      cpu->reg_PC ++;
}

/** D0 : Bxx - Branch if Not Equal**/
INSTRUCTION(BNErE)
{
   TRACEi(("BNE $%04X", cpu->reg_PC + (signed char) cpu->memory_opcode_read(cpu->reg_PC) + 1));
   if (!(cpu->reg_P & Q6502_Z_FLAG))
   {
      cpu->reg_PC += (signed char) cpu->memory_opcode_read(cpu->reg_PC ++) + 1;
      /* Need to set timing */

      /* +1 is same page */
      cpu->cycle_done += 1;
      /* +2 is another */
   }
   else
      cpu->reg_PC ++;
}

/** 10 : BPL - Branch if PLus **/
INSTRUCTION(BPLrE)
{
   TRACEi(("BPL $%04X", cpu->reg_PC + (signed char) cpu->memory_opcode_read(cpu->reg_PC) + 1));
   if (!(cpu->reg_P & Q6502_N_FLAG))
   {
      cpu->reg_PC += (signed char) cpu->memory_opcode_read(cpu->reg_PC ++) + 1;
      /* Need to set timing */

      /* +1 is same page */
      cpu->cycle_done += 1;
      /* +2 is another */
   }
   else
      cpu->reg_PC ++;
}

/** 50 : BVC - Branch if oVerflow Clear**/
INSTRUCTION(BVCrE)
{
   TRACEi(("BVC $%04X", cpu->reg_PC + (signed char) cpu->memory_opcode_read(cpu->reg_PC) + 1));
   if (!(cpu->reg_P & Q6502_V_FLAG))
   {
      cpu->reg_PC += (signed char) cpu->memory_opcode_read(cpu->reg_PC ++) + 1;
      /* Need to set timing */

      /* +1 is same page */
      cpu->cycle_done += 1;
      /* +2 is another */
   }
   else
      cpu->reg_PC ++;
}

/** 70 : BVS - Branch if oVerflow Set**/
INSTRUCTION(BVSrE)
{
   TRACEi(("BVS $%04X", cpu->reg_PC + (signed char) cpu->memory_opcode_read(cpu->reg_PC) + 1));
   if (cpu->reg_P & Q6502_V_FLAG)
   {
      cpu->reg_PC += (signed char) cpu->memory_opcode_read(cpu->reg_PC ++) + 1;
      /* Need to set timing */

      /* +1 is same page */
      cpu->cycle_done += 1;
      /* +2 is another */
   }
   else
      cpu->reg_PC ++;
}

/*** Mathematical functions ***/

/** 69 : ADC - ADd with Carry **/
INSTRUCTION(ADCiM)
{
   TRACEi(("ADC #$%02X", cpu->memory_opcode_read(cpu->reg_PC) ));
   ADC_OPERATION(cpu->memory_opcode_read(cpu->reg_PC++));
}

/** 65 : ADC - ADd with Carry **/
INSTRUCTION(ADCzP)
{
   TRACEi(("ADC $%02X", cpu->memory_opcode_read(cpu->reg_PC) ));
   ADC_OPERATION(MEMORY_READ_ZP());
}

/** 75 : ADC - ADd with Carry **/
INSTRUCTION(ADCzX)
{
   TRACEi(("ADC $%02X,X", cpu->memory_opcode_read(cpu->reg_PC) ));
   ADC_OPERATION(MEMORY_READ_ZX());
}

/** 61 : ADC - ADd with Carry **/
INSTRUCTION(ADCiX)
{
   TRACEi(("ADC ($%02X,X)", cpu->memory_opcode_read(cpu->reg_PC) ));
   ADC_OPERATION(MEMORY_READ_IX());
}

/** 71 : ADC - ADd with Carry **/
INSTRUCTION(ADCiY)
{
   TRACEi(("ADC ($%02X),Y", cpu->memory_opcode_read(cpu->reg_PC) ));
   ADC_OPERATION(MEMORY_READ_IY());
   CROSS_CYCLE_UPDATE(cpu->memory_opcode_read(cpu->reg_PC-1) + cpu->reg_Y);
}

/** 6D : ADC - ADd with Carry **/
INSTRUCTION(ADCaB)
{
   TRACEi(("ADC $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   ADC_OPERATION(MEMORY_READ_AB());
}

/** 7D : ADC - ADd with Carry **/
INSTRUCTION(ADCaX)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   
   TRACEi(("ADC $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   ADC_OPERATION(MEMORY_READ_AX());
   CROSS_CYCLE_UPDATE(op1 + cpu->reg_X);
}

/** 79 : ADC - ADd with Carry **/
INSTRUCTION(ADCaY)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   TRACEi(("ADC $%02X%02X,Y", op2, op1));
   ADC_OPERATION(MEMORY_READ_AY());
   CROSS_CYCLE_UPDATE(op1 + cpu->reg_Y);
}

/** E9 : SBC - SuBstract with Carry **/
INSTRUCTION(SBCiM)
{
   TRACEi(("SBC #$%02X", cpu->memory_opcode_read(cpu->reg_PC) ));
   SBC_OPERATION(cpu->memory_opcode_read(cpu->reg_PC++));
}

/** E5 : SBC - SuBstract with Carry **/
INSTRUCTION(SBCzP)
{
   TRACEi(("SBC $%02X", cpu->memory_opcode_read(cpu->reg_PC) ));
   SBC_OPERATION(MEMORY_READ_ZP());
}

/** F5 : SBC - SuBstract with Carry **/
INSTRUCTION(SBCzX)
{
   TRACEi(("SBC $%02X,X", cpu->memory_opcode_read(cpu->reg_PC) ));
   SBC_OPERATION(MEMORY_READ_ZX());
}

/** E1 : SBC - SuBstract with Carry **/
INSTRUCTION(SBCiX)
{
   TRACEi(("SBC ($%02X,X)", cpu->memory_opcode_read(cpu->reg_PC) ));
   SBC_OPERATION(MEMORY_READ_IX());
}

/** F1 : SBC - SuBstract with Carry **/
INSTRUCTION(SBCiY)
{
   TRACEi(("SBC ($%02X),Y", cpu->memory_opcode_read(cpu->reg_PC) ));
   SBC_OPERATION(MEMORY_READ_IY());
   CROSS_CYCLE_UPDATE(cpu->memory_opcode_read(cpu->reg_PC-1) + cpu->reg_Y);
}

/** ED : SBC - SuBstract with Carry **/
INSTRUCTION(SBCaB)
{
   TRACEi(("SBC $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   SBC_OPERATION(MEMORY_READ_AB());
}

/** FD : SBC - SuBstract with Carry **/
INSTRUCTION(SBCaX)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   
   TRACEi(("SBC $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   SBC_OPERATION(MEMORY_READ_AX());
   
   CROSS_CYCLE_UPDATE(op1 + cpu->reg_X);
}

/** F9 : SBC - SuBstract with Carry **/
INSTRUCTION(SBCaY)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   TRACEi(("SBC $%02X%02X,Y", op2, op1));
   SBC_OPERATION(MEMORY_READ_AY());
   CROSS_CYCLE_UPDATE(op1 + cpu->reg_Y);
}

/** C9 : CMP - CoMPare **/
INSTRUCTION(CMPiM)
{
   TRACEi(("CMP #$%02X", cpu->memory_opcode_read(cpu->reg_PC) ));
   CMP_OPERATION(cpu->reg_A, cpu->memory_opcode_read(cpu->reg_PC++));
}

/** C5 : CMP - CoMPare **/
INSTRUCTION(CMPzP)
{
   TRACEi(("CMP $%02X", cpu->memory_opcode_read(cpu->reg_PC) ));
   CMP_OPERATION(cpu->reg_A, MEMORY_READ_ZP());
}

/** D5 : CMP - CoMPare **/
INSTRUCTION(CMPzX)
{
   TRACEi(("CMP $%02X,X", cpu->memory_opcode_read(cpu->reg_PC) ));
   CMP_OPERATION(cpu->reg_A, MEMORY_READ_ZX());
}

/** C1 : CMP - CoMPare **/
INSTRUCTION(CMPiX)
{
   TRACEi(("CMP ($%02X,X)", cpu->memory_opcode_read(cpu->reg_PC) ));
   CMP_OPERATION(cpu->reg_A, MEMORY_READ_IX());
}

/** D1 : CMP - CoMPare **/
INSTRUCTION(CMPiY)
{
   TRACEi(("CMP ($%02X),Y", cpu->memory_opcode_read(cpu->reg_PC) ));
   CMP_OPERATION(cpu->reg_A, MEMORY_READ_IY());
   CROSS_CYCLE_UPDATE(cpu->memory_opcode_read(cpu->reg_PC-1) + cpu->reg_Y);
}

/** CD : CMP - CoMPare **/
INSTRUCTION(CMPaB)
{
   TRACEi(("CMP $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   CMP_OPERATION(cpu->reg_A, MEMORY_READ_AB());
}

/** DD : CMP - CoMPare **/
INSTRUCTION(CMPaX)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   
   TRACEi(("CMP $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   CMP_OPERATION(cpu->reg_A, MEMORY_READ_AX());
   CROSS_CYCLE_UPDATE(op1 + cpu->reg_X);
}

/** D9 : CMP - CoMPare **/
INSTRUCTION(CMPaY)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   TRACEi(("CMP $%02X%02X,Y", op2, op1));
   CMP_OPERATION(cpu->reg_A, MEMORY_READ_AY());
   CROSS_CYCLE_UPDATE(op1 + cpu->reg_Y);
}

/** E0 : CPX - ComPare with Y **/
INSTRUCTION(CPXiM)
{
   TRACEi(("CPX #$%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   CMP_OPERATION(cpu->reg_X, cpu->memory_opcode_read(cpu->reg_PC++));
}

/** E4 : CPX - ComPare with X **/
INSTRUCTION(CPXzP)
{
   TRACEi(("CPX $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   CMP_OPERATION(cpu->reg_X, MEMORY_READ_ZP());
}

/** EC : CPX - ComPare with X **/
INSTRUCTION(CPXaB)
{
   TRACEi(("CPX $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   CMP_OPERATION(cpu->reg_X, MEMORY_READ_AB());
}

/** C0 : CPY - ComPare with Y **/
INSTRUCTION(CPYiM)
{
   TRACEi(("CPY #$%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   CMP_OPERATION(cpu->reg_Y, cpu->memory_opcode_read(cpu->reg_PC++));
}

/** C4 : CPY - ComPare with Y **/
INSTRUCTION(CPYzP)
{
   TRACEi(("CPY $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   CMP_OPERATION(cpu->reg_Y, MEMORY_READ_ZP());
}

/** CC : CPY - ComPare with Y **/
INSTRUCTION(CPYaB)
{
   TRACEi(("CPY $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   CMP_OPERATION(cpu->reg_Y, MEMORY_READ_AB());
}

/** 09 : ORA - OR with A **/
INSTRUCTION(ORAiM)
{
   TRACEi(("ORA #$%02X", cpu->memory_opcode_read(cpu->reg_PC) ));
   ORA_OPERATION(cpu->memory_opcode_read(cpu->reg_PC++));
}

/** 405 : ORA - OR with A **/
INSTRUCTION(ORAzP)
{
   TRACEi(("ORA $%02X", cpu->memory_opcode_read(cpu->reg_PC) ));
   ORA_OPERATION(cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC++)));
}

/** 15 : ORA - OR with A **/
INSTRUCTION(ORAzX)
{
   TRACEi(("ORA $%02X,X", cpu->memory_opcode_read(cpu->reg_PC) ));
   ORA_OPERATION(MEMORY_READ_ZX());
}

/** 01 : ORA - OR with A **/
INSTRUCTION(ORAiX)
{
   TRACEi(("ORA ($%02X,X)", cpu->memory_opcode_read(cpu->reg_PC) ));
   ORA_OPERATION(MEMORY_READ_IX());
}

/** 11 : ORA - OR with A **/
INSTRUCTION(ORAiY)
{
   TRACEi(("ORA ($%02X),Y", cpu->memory_opcode_read(cpu->reg_PC) ));
   ORA_OPERATION(MEMORY_READ_IY());
   CROSS_CYCLE_UPDATE(cpu->memory_opcode_read(cpu->reg_PC-1) + cpu->reg_Y);
}

/** 0D : ORA - OR with A **/
INSTRUCTION(ORAaB)
{
   TRACEi(("ORA $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   ORA_OPERATION(MEMORY_READ_AB());
}

/** 1D : ORA - OR with A **/
INSTRUCTION(ORAaX)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++); 
   
   TRACEi(("ORA $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   ORA_OPERATION(MEMORY_READ_AX());
   CROSS_CYCLE_UPDATE(op1 + cpu->reg_X);
}

/** 19 : ORA - OR with A **/
INSTRUCTION(ORAaY)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   
   TRACEi(("ORA $%02X%02X,Y", op2, op1));
   ORA_OPERATION(MEMORY_READ_AY());
   CROSS_CYCLE_UPDATE(op1 + cpu->reg_Y);
}

/** 49 : EOR - Exclusive OR **/
INSTRUCTION(EORiM)
{
   TRACEi(("EOR #$%02X", cpu->memory_opcode_read(cpu->reg_PC) ));
   EOR_OPERATION(cpu->memory_opcode_read(cpu->reg_PC++));
}

/** 45 : EOR - Exclusive OR **/
INSTRUCTION(EORzP)
{
   TRACEi(("EOR $%02X", cpu->memory_opcode_read(cpu->reg_PC) ));
   EOR_OPERATION(cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC++)));
}

/** 55 : EOR - Exclusive OR **/
INSTRUCTION(EORzX)
{
   TRACEi(("EOR $%02X,X", cpu->memory_opcode_read(cpu->reg_PC) ));
   EOR_OPERATION(MEMORY_READ_ZX());
}

/** 41 : EOR - Exclusive OR **/
INSTRUCTION(EORiX)
{
   TRACEi(("EOR ($%02X,X)", cpu->memory_opcode_read(cpu->reg_PC) ));
   EOR_OPERATION(MEMORY_READ_IX());
}

/** 51 : EOR - Exclusive OR **/
INSTRUCTION(EORiY)
{
   TRACEi(("EOR ($%02X),Y", cpu->memory_opcode_read(cpu->reg_PC) ));
   EOR_OPERATION(MEMORY_READ_IY());
   
   CROSS_CYCLE_UPDATE(cpu->memory_opcode_read(cpu->reg_PC-1) + cpu->reg_Y);
}

/** 4D : EOR - Exclusive OR **/
INSTRUCTION(EORaB)
{
   TRACEi(("EOR $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   EOR_OPERATION(MEMORY_READ_AB());
}

/** 5D : EOR - Exclusive OR **/
INSTRUCTION(EORaX)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   
   TRACEi(("EOR $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   EOR_OPERATION(MEMORY_READ_AX());
   CROSS_CYCLE_UPDATE(op1 + cpu->reg_X);
}

/** 59 : EOR - Exclusive OR **/
INSTRUCTION(EORaY)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   TRACEi(("EOR $%02X%02X,Y", op2, op1));
   EOR_OPERATION(MEMORY_READ_AY());
   CROSS_CYCLE_UPDATE(op1 + cpu->reg_Y);
}

/** 29 : AND - Logical AND **/
INSTRUCTION(ANDiM)
{
   TRACEi(("AND #$%02X", cpu->memory_opcode_read(cpu->reg_PC) ));
   AND_OPERATION(cpu->memory_opcode_read(cpu->reg_PC++));
}

/** 25 : AND - Logical AND **/
INSTRUCTION(ANDzP)
{
   TRACEi(("AND $%02X", cpu->memory_opcode_read(cpu->reg_PC) ));
   AND_OPERATION(cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC++)));
}

/** 35 : AND - Logical AND **/
INSTRUCTION(ANDzX)
{
   TRACEi(("AND $%02X,X", cpu->memory_opcode_read(cpu->reg_PC) ));
   AND_OPERATION(MEMORY_READ_ZX());
}

/** 21 : AND - Logical AND **/
INSTRUCTION(ANDiX)
{
   TRACEi(("AND ($%02X,X)", cpu->memory_opcode_read(cpu->reg_PC) ));
   AND_OPERATION(MEMORY_READ_IX());
}

/** 31 : AND - Logical AND **/
INSTRUCTION(ANDiY)
{
   TRACEi(("AND ($%02X),Y", cpu->memory_opcode_read(cpu->reg_PC) ));
   AND_OPERATION(MEMORY_READ_IY());
   
   CROSS_CYCLE_UPDATE(cpu->memory_opcode_read(cpu->reg_PC-1) + cpu->reg_Y);
}

/** 2D : AND - Logical AND **/
INSTRUCTION(ANDaB)
{
   TRACEi(("AND $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   AND_OPERATION(MEMORY_READ_AB());
}

/** 3D : AND - Logical AND **/
INSTRUCTION(ANDaX)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   
   TRACEi(("AND $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   AND_OPERATION(MEMORY_READ_AX());
   
   CROSS_CYCLE_UPDATE(op1 + cpu->reg_X);
}

/** 39 : AND - Logical AND **/
INSTRUCTION(ANDaY)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   TRACEi(("AND $%02X%02X,Y", op2, op1));
   AND_OPERATION(MEMORY_READ_AY());
   CROSS_CYCLE_UPDATE(op1 + cpu->reg_Y);
}

/*** Misc instructions ***/

/** 24 : BIT **/
INSTRUCTION(BITzP)
{
   TRACEi(("BIT $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   BIT_OPERATION(MEMORY_READ_ZP());
}

/** 2C : BIT **/
INSTRUCTION(BITaB)
{
   TRACEi(("BIT $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   BIT_OPERATION(MEMORY_READ_AB());
}

/** 2A : ROL A **/
INSTRUCTION(ROLiM)
{
   TRACEi(("ROL A"));
   ROL_OPERATION(cpu->reg_A);
}

/** 6A : ROR A **/
INSTRUCTION(RORiM)
{
   TRACEi(("ROR A"));
   ROR_OPERATION(cpu->reg_A);
}

/** 0A : ASL A **/
INSTRUCTION(ASLiM)
{
   TRACEi(("ASL A"));
   ASL_OPERATION(cpu->reg_A);
}

/** 4A : LSR A **/
INSTRUCTION(LSRiM)
{
   TRACEi(("LSR A"));
   LSR_OPERATION(cpu->reg_A);
}

/** 2E : ROL **/
INSTRUCTION(ROLaB)
{
   TRACEi(("ROL $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_AB();
   cpu->reg_PC -= 2;
   ROL_OPERATION(val);
   MEMORY_WRITE_AB(val);
}

/** 26 : ROL **/
INSTRUCTION(ROLzP)
{
   TRACEi(("ROL $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_ZP();
   cpu->reg_PC -= 1;
   ROL_OPERATION(val);
   MEMORY_WRITE_ZP(val);
}

/** 3E : ROL **/
INSTRUCTION(ROLaX)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   
   TRACEi(("ROL $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_AX();
   cpu->reg_PC -= 2;
   ROL_OPERATION(val);
   MEMORY_WRITE_AX(val);
}

/** 36 : ROL **/
INSTRUCTION(ROLzX)
{
   TRACEi(("ROL $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_ZX();
   cpu->reg_PC -= 1;
   ROL_OPERATION(val);
   MEMORY_WRITE_ZX(val);
}

/** 6E : ROR **/
INSTRUCTION(RORaB)
{
   TRACEi(("ROR $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_AB();
   cpu->reg_PC -= 2;
   ROR_OPERATION(val);
   MEMORY_WRITE_AB(val);
}

/** 66 : ROR **/
INSTRUCTION(RORzP)
{
   TRACEi(("ROR $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_ZP();
   cpu->reg_PC -= 1;
   ROR_OPERATION(val);
   MEMORY_WRITE_ZP(val);
}

/** 7E : ROR **/
INSTRUCTION(RORaX)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   
   TRACEi(("ROR $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_AX();
   cpu->reg_PC -= 2;
   ROR_OPERATION(val);
   MEMORY_WRITE_AX(val);
}

/** 76 : ROR **/
INSTRUCTION(RORzX)
{
   TRACEi(("ROR $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_ZX();
   cpu->reg_PC -= 1;
   ROR_OPERATION(val);
   MEMORY_WRITE_ZX(val);
}

/** 0E : ASL **/
INSTRUCTION(ASLaB)
{
   TRACEi(("ASL $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_AB();
   cpu->reg_PC -= 2;
   ASL_OPERATION(val);
   MEMORY_WRITE_AB(val);
}

/** 06 : ASL **/
INSTRUCTION(ASLzP)
{
   TRACEi(("ASL $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_ZP();
   cpu->reg_PC -= 1;
   ASL_OPERATION(val);
   MEMORY_WRITE_ZP(val);
}

/** 1E : ASL **/
INSTRUCTION(ASLaX)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   
   TRACEi(("ASL $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_AX();
   cpu->reg_PC -= 2;
   ASL_OPERATION(val);
   MEMORY_WRITE_AX(val);
}

/** 16 : ASL **/
INSTRUCTION(ASLzX)
{
   TRACEi(("ASL $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_ZX();
   cpu->reg_PC -= 1;
   ASL_OPERATION(val);
   MEMORY_WRITE_ZX(val);
}

/** 4E : LSR **/
INSTRUCTION(LSRaB)
{
   TRACEi(("LSR $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_AB();
   cpu->reg_PC -= 2;
   LSR_OPERATION(val);
   MEMORY_WRITE_AB(val);
}

/** 46 : LSR **/
INSTRUCTION(LSRzP)
{
   TRACEi(("LSR $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_ZP();
   cpu->reg_PC -= 1;
   LSR_OPERATION(val);
   MEMORY_WRITE_ZP(val);
}

/** 5E : LSR **/
INSTRUCTION(LSRaX)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   
   TRACEi(("LSR $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_AX();
   cpu->reg_PC -= 2;
   LSR_OPERATION(val);
   MEMORY_WRITE_AX(val);
}

/** 56 : LSR **/
INSTRUCTION(LSRzX)
{
   TRACEi(("LSR $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_ZX();
   cpu->reg_PC -= 1;
   LSR_OPERATION(val);
   MEMORY_WRITE_ZX(val);
}

/** CE : DEC **/
INSTRUCTION(DECaB)
{
   TRACEi(("DEC $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_AB();
   cpu->reg_PC -= 2;
   MEMORY_WRITE_AB(--val);
   NZ_FLAG_UPDATE(val);
}

/** C6 : DEC **/
INSTRUCTION(DECzP)
{
   TRACEi(("DEC $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_ZP();
   cpu->reg_PC -= 1;
   MEMORY_WRITE_ZP(--val);
   NZ_FLAG_UPDATE(val);
}

/** DE : DEC **/
INSTRUCTION(DECaX)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   
   TRACEi(("DEC $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_AX();
   cpu->reg_PC -= 2;
   MEMORY_WRITE_AX(--val);
   NZ_FLAG_UPDATE(val);
}

/** D6 : DEC **/
INSTRUCTION(DECzX)
{
   TRACEi(("DEC $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_ZX();
   cpu->reg_PC -= 1;
   MEMORY_WRITE_ZX(--val);
   NZ_FLAG_UPDATE(val);
}

/** EE : INC **/
INSTRUCTION(INCaB)
{
   TRACEi(("INC $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_AB();
   cpu->reg_PC -= 2;
   MEMORY_WRITE_AB(++val);
   NZ_FLAG_UPDATE(val);
}

/** E6 : INC **/
INSTRUCTION(INCzP)
{
   TRACEi(("INC $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_ZP();
   cpu->reg_PC -= 1;
   MEMORY_WRITE_ZP(++val);
   NZ_FLAG_UPDATE(val);
}

/** FE : INC **/
INSTRUCTION(INCaX)
{
   register byte op1 = cpu->memory_opcode_read(cpu->reg_PC++);
   register byte op2 = cpu->memory_opcode_read(cpu->reg_PC++);
   
   TRACEi(("INC $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC+1), cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_AX();
   cpu->reg_PC -= 2;
   MEMORY_WRITE_AX(++val);
   NZ_FLAG_UPDATE(val);
}
 
/** F6 : INC **/
INSTRUCTION(INCzX)
{
   TRACEi(("INC $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
   byte val = MEMORY_READ_ZX();
   cpu->reg_PC -= 1;
   MEMORY_WRITE_ZX(++val);
   NZ_FLAG_UPDATE(val);
}

/* */
static InstructionFunction InstructionTable[256] =
{
/*         00       01       02       03       04       05       06       07       08       09       0A       0B       0C       0D       0E       0F */
/* 00 */ I_BRKiM, I_ORAiX, I_ILLEG, I_ILLEG, I_ILLEG, I_ORAzP, I_ASLzP, I_ILLEG, I_PHPiM, I_ORAiM, I_ASLiM, I_ILLEG, I_ILLEG, I_ORAaB, I_ASLaB, I_ILLEG,
/* 10 */ I_BPLrE, I_ORAiY, I_ILLEG, I_ILLEG, I_ILLEG, I_ORAzX, I_ASLzX, I_ILLEG, I_CLCiM, I_ORAaY, I_ILLEG, I_ILLEG, I_ILLEG, I_ORAaX, I_ASLaX, I_ILLEG,
/* 20 */ I_JSRaB, I_ANDiX, I_ILLEG, I_ILLEG, I_BITzP, I_ANDzP, I_ROLzP, I_ILLEG, I_PLPiM, I_ANDiM, I_ROLiM, I_ILLEG, I_BITaB, I_ANDaB, I_ROLaB, I_ILLEG,
/* 30 */ I_BMIrE, I_ANDiY, I_ILLEG, I_ILLEG, I_ILLEG, I_ANDzX, I_ROLzX, I_ILLEG, I_SECiM, I_ANDaY, I_ILLEG, I_ILLEG, I_ILLEG, I_ANDaX, I_ROLaX, I_ILLEG,
/* 40 */ I_RTIiM, I_EORiX, I_ILLEG, I_ILLEG, I_ILLEG, I_EORzP, I_LSRzP, I_ILLEG, I_PHAiM, I_EORiM, I_LSRiM, I_ILLEG, I_JMPaB, I_EORaB, I_LSRaB, I_ILLEG,
/* 50 */ I_BVCrE, I_EORiY, I_ILLEG, I_ILLEG, I_ILLEG, I_EORzX, I_LSRzX, I_ILLEG, I_CLIiM, I_EORaY, I_ILLEG, I_ILLEG, I_ILLEG, I_EORaX, I_LSRaX, I_ILLEG,
/* 60 */ I_RTSiM, I_ADCiX, I_ILLEG, I_ILLEG, I_ILLEG, I_ADCzP, I_RORzP, I_ILLEG, I_PLAiM, I_ADCiM, I_RORiM, I_ILLEG, I_JMPiD, I_ADCaB, I_RORaB, I_ILLEG,
/* 70 */ I_BVSrE, I_ADCiY, I_ILLEG, I_ILLEG, I_ILLEG, I_ADCzX, I_RORzX, I_ILLEG, I_SEIiM, I_ADCaY, I_ILLEG, I_ILLEG, I_ILLEG, I_ADCaX, I_RORaX, I_ILLEG,
/* 80 */ I_ILLEG, I_STAiX, I_ILLEG, I_ILLEG, I_STYzP, I_STAzP, I_STXzP, I_ILLEG, I_DEYiM, I_ILLEG, I_TXAiM, I_ILLEG, I_STYaB, I_STAaB, I_STXaB, I_ILLEG,
/* 90 */ I_BCCrE, I_STAiY, I_ILLEG, I_ILLEG, I_STYzX, I_STAzX, I_STXzY, I_ILLEG, I_TYAiM, I_STAaY, I_TXSiM, I_ILLEG, I_ILLEG, I_STAaX, I_ILLEG, I_ILLEG,
/* A0 */ I_LDYiM, I_LDAiX, I_LDXiM, I_ILLEG, I_LDYzP, I_LDAzP, I_LDXzP, I_ILLEG, I_TAYiM, I_LDAiM, I_TAXiM, I_ILLEG, I_LDYaB, I_LDAaB, I_LDXaB, I_ILLEG,
/* B0 */ I_BCSrE, I_LDAiY, I_ILLEG, I_ILLEG, I_LDYzX, I_LDAzX, I_LDXzY, I_ILLEG, I_CLViM, I_LDAaY, I_TSXiM, I_ILLEG, I_LDYaX, I_LDAaX, I_LDXaY, I_ILLEG,
/* C0 */ I_CPYiM, I_CMPiX, I_ILLEG, I_ILLEG, I_CPYzP, I_CMPzP, I_DECzP, I_ILLEG, I_INYiM, I_CMPiM, I_DEXiM, I_ILLEG, I_CPYaB, I_CMPaB, I_DECaB, I_ILLEG,
/* D0 */ I_BNErE, I_CMPiY, I_ILLEG, I_ILLEG, I_ILLEG, I_CMPzX, I_DECzX, I_ILLEG, I_CLDiM, I_CMPaY, I_ILLEG, I_ILLEG, I_ILLEG, I_CMPaX, I_DECaX, I_ILLEG,
/* E0 */ I_CPXiM, I_SBCiX, I_ILLEG, I_ILLEG, I_CPXzP, I_SBCzP, I_INCzP, I_ILLEG, I_INXiM, I_SBCiM, I_NOPiM, I_ILLEG, I_CPXaB, I_SBCaB, I_INCaB, I_ILLEG,
/* F0 */ I_BEQrE, I_SBCiY, I_ILLEG, I_ILLEG, I_ILLEG, I_SBCzX, I_INCzX, I_ILLEG, I_SEDiM, I_SBCaY, I_ILLEG, I_ILLEG, I_ILLEG, I_SBCaX, I_INCaX, I_ILLEG
/*         00       01       02       03       04       05       06       07       08       09       0A       0B       0C       0D       0E       0F */
};


static inline int quick6502_exec_one(quick6502_cpu *cpu)
{
   register byte opcode = cpu->memory_opcode_read(cpu->reg_PC++);
   
   TRACEi(("Quick6502: PC:$%04X A:$%02X X:$%02X Y:$%02X S:$%02X P:$%02X P:[%c%c%c%c%c%c%c%c]",
           cpu->reg_PC, cpu->reg_A, cpu->reg_X, cpu->reg_Y, cpu->reg_S, cpu->reg_P,
           cpu->reg_P&Q6502_N_FLAG ? 'N':'.',
           cpu->reg_P&Q6502_V_FLAG ? 'V':'.',
           cpu->reg_P&Q6502_R_FLAG ? 'R':'.',
           cpu->reg_P&Q6502_B_FLAG ? 'B':'.',
           cpu->reg_P&Q6502_D_FLAG ? 'D':'.',
           cpu->reg_P&Q6502_I_FLAG ? 'I':'.',
           cpu->reg_P&Q6502_Z_FLAG ? 'Z':'.',
           cpu->reg_P&Q6502_C_FLAG ? 'C':'.'));
   
   InstructionTable[opcode](cpu);
   cpu->cycle_done += CycleTable[opcode];
   if (cpu->page_crossed) { cpu->cycle_done++; cpu->page_crossed = 0; }
   if (cpu->int_pending != 0)
   {
      quick6502_int(cpu, Q6502_IRQ_SIGNAL);
   }
   return 0;
}
