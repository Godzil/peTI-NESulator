/**
 *  CoreCPU - The Quick6502 Project
 *  corecpu.c
 *
 *  Created by Manoël Trapier on 24/02/08
 *  Copyright (c) 2002-2019 986-Studio.
 *
 */

/* TODO: Add Inst/MemAccess breakpoints */

/* Depending on the OS, one of these provide the malloc function */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <os_dependent.h>

/*******************************************************************************
 /!\ WARNING this debug tool slow down a lot the emulator!                   /!\
 /!\ Use it only if you really need it !                                     /!\
 *******************************************************************************/
//#define TRACE_INSTRUCTIONS

#ifdef TRACE_INSTRUCTIONS
#define TRACEi(trace) do { console_printf(Console_Debug, ">> $%04X - ", cpu->reg_PC-1); console_printf_d trace ; console_printf(Console_Debug, "\n"); } while(0)
#define TRACEiE(trace) do { console_printf(Console_Debug, ">> $%04X - ", cpu->reg_PC-1); console_printf_d trace ; console_printf(Console_Debug, "\n"); } while(0)
#else
#define TRACEi(trace) { }
//#define TRACEiE(trace) { }
#define TRACEiE(trace) do { console_printf(Console_Debug, ">> $%04X - ", cpu->reg_PC-1); console_printf_d trace ; console_printf(Console_Debug, "\n"); } while(0)
#endif

/*
 * IP_ == "Instruction Parameter"
 * nP: No parameters
 * iM: Immediate
 * iX: Indirect by X
 * iY: Indirect by Y
 * zP: Zero Page
 * zX: Zero Page Index by X
 * zY: Zero Page Index by Y
 * iD: Indirect Double
 * aB: Absolute
 * aX: Absolute by X
 * aY: Absolute by Y 
 * rE: Relative
 */
#define IP_nP "N"
#define IP_iM "I"
#define IP_iX "X"
#define IP_iY "Y"
#define IP_zP "0"
#define IP_zX "z"
#define IP_zY "Z"
#define IP_iD "D"
#define IP_aB "A"
#define IP_aX "x"
#define IP_aY "y"
#define IP_rE "R"

#define IP_nPc 'N'
#define IP_iMc 'I'
#define IP_iXc 'X'
#define IP_iYc 'Y'
#define IP_zPc '0'
#define IP_zXc 'z'
#define IP_zYc 'Z'
#define IP_iDc 'D'
#define IP_aBc 'A'
#define IP_aXc 'x'
#define IP_aYc 'y'
#define IP_rEc 'R'

#define IPf_nP ""
#define IPf_iM " $%02X"
#define IPf_iX " ($%02X,X)"
#define IPf_iY " ($%02X),Y"
#define IPf_zP " $%02X"
#define IPf_zX " $%02X,X"
#define IPf_zY " $%02X,Y"
#define IPf_iD " ($%02X%02X)"
#define IPf_aB " $%02X%02X"
#define IPf_aX " $%02X%02X,X"
#define IPf_aY " $%02X%02X,Y"
#define IPf_rE " $%02X%02X"

#define _INTERNAL_QUICK6502_CORECPU_

#include "corecpu.h"


/*** Instructions useful macros ***/
#define INSTRUCTION(s) static inline void I_##s (quick6502_cpu *cpu)

#define NZ_FLAG_UPDATE(value) cpu->reg_P = ((cpu->reg_P & ~(Q6502_N_FLAG | Q6502_Z_FLAG)) | \
                                            ((value) & 0x80) | ((value)?0:Q6502_Z_FLAG))

#define CROSS_CYCLE_UPDATE(value) if ((value) & 0x0F00) cpu->page_crossed = 1

#define MEMORY_READ_ZP() cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC++))

#define MEMORY_READ_IX() cpu->memory_read( (cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC  ) + cpu->reg_X    ) & 0xFF) |\
                                           (cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC  ) + cpu->reg_X + 1) << 8) ); cpu->reg_PC++
#define MEMORY_READ_IY() cpu->memory_read( ( cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC  )    ) |\
                                            (cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC  ) + 1) << 8) ) + cpu->reg_Y  ); cpu->reg_PC++

#define MEMORY_READ_ZX() cpu->memory_page0_read( (cpu->memory_opcode_read(cpu->reg_PC++) + cpu->reg_X) )
#define MEMORY_READ_ZY() cpu->memory_page0_read( (cpu->memory_opcode_read(cpu->reg_PC++) + cpu->reg_Y) )

#define MEMORY_READ_AB() cpu->memory_read( ((cpu->memory_opcode_read(cpu->reg_PC  )     ) |\
                                            (cpu->memory_opcode_read(cpu->reg_PC+1) << 8) )); cpu->reg_PC += 2

#define MEMORY_READ_AX() cpu->memory_read( ((op1     ) |\
                                            (op2 << 8) ) + cpu->reg_X)
#define MEMORY_READ_AY() cpu->memory_read( ((op1     ) |\
                                            (op2 << 8) ) + cpu->reg_Y )


#define MEMORY_WRITE_ZP(val) cpu->memory_page0_write(cpu->memory_opcode_read(cpu->reg_PC++), val)

#define MEMORY_WRITE_IX(val) cpu->memory_write( (cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC  ) + cpu->reg_X    ) & 0xFF) |\
                                                 (cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC  ) + cpu->reg_X + 1) << 8)   , val); cpu->reg_PC++
#define MEMORY_WRITE_IY(val) cpu->memory_write( ( cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC  )    ) |\
                                                 (cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC  ) + 1) << 8) ) + cpu->reg_Y , val); cpu->reg_PC++

#define MEMORY_WRITE_ZX(val) cpu->memory_page0_write( (cpu->memory_opcode_read(cpu->reg_PC++) + cpu->reg_X), val)
#define MEMORY_WRITE_ZY(val) cpu->memory_page0_write( (cpu->memory_opcode_read(cpu->reg_PC++) + cpu->reg_Y), val)

#define MEMORY_WRITE_AB(val) cpu->memory_write( ((cpu->memory_opcode_read(cpu->reg_PC  )     ) |\
                                                 (cpu->memory_opcode_read(cpu->reg_PC+1) << 8) ), val); cpu->reg_PC += 2

#define MEMORY_WRITE_AX(val) cpu->memory_write( ((cpu->memory_opcode_read(cpu->reg_PC  )     ) |\
                                                 (cpu->memory_opcode_read(cpu->reg_PC+1) << 8) ) + cpu->reg_X, val); cpu->reg_PC += 2
#define MEMORY_WRITE_AY(val) cpu->memory_write( ((cpu->memory_opcode_read(cpu->reg_PC  )     ) |\
                                                 (cpu->memory_opcode_read(cpu->reg_PC+1) << 8) ) + cpu->reg_Y, val); cpu->reg_PC += 2


#define PUSH_S(value) cpu->memory_stack_write(0x100 | (cpu->reg_S--), value)
#define POP_S()      (cpu->memory_stack_read (0x100 | (++cpu->reg_S)       ))

#ifdef Q6502_NO_DECIMAL

#define ADC_OPERATION(read) do {\
   uint16_t tmp = 0; uint8_t v = read; \
   tmp = cpu->reg_A + v + (cpu->reg_P & Q6502_C_FLAG); \
   cpu->reg_P = (cpu->reg_P & ~(Q6502_C_FLAG | Q6502_N_FLAG | Q6502_Z_FLAG | Q6502_V_FLAG)) | \
          (tmp & Q6502_N_FLAG) | ((tmp&0xFF)?0:Q6502_Z_FLAG) | \
         ((tmp & 0xFF00)?Q6502_C_FLAG:0) | \
          ( (( ~(cpu->reg_A^v)&(cpu->reg_A^tmp) )&0x80)?Q6502_V_FLAG:0 ); \
   cpu->reg_A = tmp & 0xFF; \
} while(0)

#define SBC_OPERATION(read) do {\
   uint16_t tmp = 0; uint8_t v = read; \
   tmp = cpu->reg_A - v - (~cpu->reg_P & Q6502_C_FLAG); \
   cpu->reg_P = (cpu->reg_P & ~(Q6502_C_FLAG | Q6502_N_FLAG | Q6502_Z_FLAG | Q6502_V_FLAG)) | \
          (tmp & Q6502_N_FLAG) | ((tmp&0xFF)?0:Q6502_Z_FLAG) | \
         ((tmp & 0xFF00)?0:Q6502_C_FLAG) | \
          ( (( (cpu->reg_A^v)&(cpu->reg_A^tmp) )&0x80)?Q6502_V_FLAG:0 ); \
   cpu->reg_A = tmp & 0xFF; \
} while(0)

#else
#error Quick6502 doesn t actually support DECIMAL mode
#endif


#define AND_OPERATION(read) cpu->reg_A &= read; NZ_FLAG_UPDATE(cpu->reg_A)

/* CMP is like SBC but without storing the result value */
#define CMP_OPERATION(register, read) do { \
   uint16_t tmp = 0; \
   tmp = register - read; \
   cpu->reg_P = (cpu->reg_P & ~(Q6502_C_FLAG | Q6502_N_FLAG | Q6502_Z_FLAG)) | \
          (tmp & Q6502_N_FLAG) | ((tmp&0xFF)?0:Q6502_Z_FLAG) | \
          ((tmp & 0xFF00)?0:Q6502_C_FLAG); \
} while(0)

#define EOR_OPERATION(read) cpu->reg_A ^= read; NZ_FLAG_UPDATE(cpu->reg_A)
#define ORA_OPERATION(read) cpu->reg_A |= read; NZ_FLAG_UPDATE(cpu->reg_A)

#define BIT_OPERATION(read) do { \
   uint8_t tmp = read; \
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
   uint16_t tmp = val | (cpu->reg_P & Q6502_C_FLAG) << 8; \
   cpu->reg_P = (cpu->reg_P & ~(Q6502_C_FLAG | Q6502_N_FLAG | Q6502_Z_FLAG)) | \
          ( tmp&Q6502_C_FLAG) |         /* Set the C flag */ \
          ((tmp&0x100) >> 1) |          /* Set the N flag */ \
          ((tmp&0x1FE)?0:Q6502_Z_FLAG); /* 0x1FE will be the new 8bit value */ \
   val = (tmp>>1) & 0xFF; \
} while(0)

#define ROL_OPERATION(val) do {\
uint16_t tmp = (val << 1) | (cpu->reg_P & Q6502_C_FLAG); \
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
 *           NULL if an error occurred !
 */
quick6502_cpu *quick6502_init(quick6502_cpuconfig *config)
{
    quick6502_cpu *cpu;

    /* Alloc structure */
    cpu = (quick6502_cpu *)malloc(sizeof(quick6502_cpu));
    if (!cpu)
    {
        return NULL;
    }

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
    {
        cpu->memory_read = config->memory_read;
    }
    else
    {
        goto init_error;
    }

    if (config->memory_write != NULL)
    {
        cpu->memory_write = config->memory_write;
    }
    else
    {
        goto init_error;
    }

    if (config->memory_opcode_read != NULL)
    {
        cpu->memory_opcode_read = config->memory_opcode_read;
    }
    else
    {
        cpu->memory_opcode_read = config->memory_read;
    }


    if (config->memory_page0_read != NULL)
    {
        cpu->memory_page0_read = config->memory_page0_read;
    }
    else
    {
        cpu->memory_page0_read = config->memory_read;
    }

    if (config->memory_page0_write != NULL)
    {
        cpu->memory_page0_write = config->memory_page0_write;
    }
    else
    {
        cpu->memory_page0_write = config->memory_write;
    }

    if (config->memory_stack_read != NULL)
    {
        cpu->memory_stack_read = config->memory_stack_read;
    }
    else
    {
        cpu->memory_stack_read = config->memory_read;
    }

    if (config->memory_stack_write != NULL)
    {
        cpu->memory_stack_write = config->memory_stack_write;
    }
    else
    {
        cpu->memory_stack_write = config->memory_write;
    }

    return cpu;

init_error:
    if (cpu)
    {
        free(cpu);
    }

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
    cpu->reg_PC = (cpu->memory_read(Q6502_RESET_HIGH) << 8)
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
uint32_t quick6502_run(quick6502_cpu *cpu, uint32_t cycles)
{
    cpu->running = true;

    while (cpu->cycle_done < cycles)
    {
        quick6502_exec_one(cpu);
    }
    cpu->cycle_done -= cycles;

    cpu->running = false;

    return cycles + cpu->cycle_done;
}

/** Loop CPU until explicit quit */
void quick6502_loop(quick6502_cpu *cpu)
{
    cpu->running = true;
    while (cpu->exit_loop)
    {
        quick6502_exec_one(cpu);
    }
    cpu->running = false;
}

/** Run CPU for one instruction */
void quick6502_exec(quick6502_cpu *cpu)
{
    cpu->running = true;
    quick6502_exec_one(cpu);
    cpu->running = false;
}

/** Send IRQ/NMI/EXITLOOP signal to CPU */
void quick6502_int(quick6502_cpu *cpu, quick6502_signal signal)
{
    switch (signal)
    {
    default:
        break;

    case Q6502_IRQ_SIGNAL:
        if (!(cpu->reg_P & Q6502_I_FLAG))
        {
            TRACEi(("IRQ Triggered !"));
            PUSH_S((cpu->reg_PC >> 8) & 0xFF);
            PUSH_S((cpu->reg_PC) & 0xFF);
            PUSH_S(cpu->reg_P & ~Q6502_B_FLAG);
            cpu->reg_P = cpu->reg_P | Q6502_I_FLAG;

            cpu->reg_PC = (cpu->memory_read(Q6502_IRQ_LOW)) | (cpu->memory_read(Q6502_IRQ_HIGH) << 8);

            cpu->cycle_done += 7;
        }
        else
        {
            cpu->int_pending = 1;
        }

        break;

    case Q6502_NMI_SIGNAL:
    TRACEi(("NMI Triggered !"));
        PUSH_S((cpu->reg_PC >> 8) & 0xFF);
        PUSH_S((cpu->reg_PC) & 0xFF);
        PUSH_S(cpu->reg_P);
        cpu->reg_P = (cpu->reg_P | Q6502_I_FLAG) & ~Q6502_B_FLAG;

        cpu->reg_PC = (cpu->memory_read(Q6502_NMI_LOW)) | (cpu->memory_read(Q6502_NMI_HIGH) << 8);

        cpu->cycle_done += 7;
        break;

    case Q6502_STOPLOOP_SIGNAL:
        cpu->exit_loop = 1;
        break;
    }
}

/** Dump CPU State to the given file */
void quick6502_dump(quick6502_cpu *cpu, FILE *fp)
{
    short i;
    char instr[100];
    /* Display registers */
    fprintf(fp,
            "## Quick6502: PC:$%04X A:$%02X X:$%02X Y:$%02X S:$%02X P:$%02X [%c%c%c%c%c%c%c%c]\n",
            cpu->reg_PC, cpu->reg_A, cpu->reg_X, cpu->reg_Y, cpu->reg_S, cpu->reg_P,
            (cpu->reg_P & Q6502_N_FLAG) ? 'N' : '.',
            (cpu->reg_P & Q6502_V_FLAG) ? 'V' : '.',
            '.', /* No real flag here */
            (cpu->reg_P & Q6502_B_FLAG) ? 'B' : '.',
            (cpu->reg_P & Q6502_D_FLAG) ? 'D' : '.',
            (cpu->reg_P & Q6502_I_FLAG) ? 'I' : '.',
            (cpu->reg_P & Q6502_Z_FLAG) ? 'Z' : '.',
            (cpu->reg_P & Q6502_C_FLAG) ? 'C' : '.'
    );

    /* Display stack */
    fprintf(fp, "## Quick6502: Stack: [ ");
    for (i = cpu->reg_S + 1 ; i < 0x100 ; i++)
    {
        fprintf(fp, "$%02X ", cpu->memory_read(0x100 | i));
    }
    fprintf(fp, "] Run:%c Cycle:%ld\n", cpu->running ? 'Y' : 'N', cpu->cycle_done);

    fprintf(fp, "## Quick6502: InstrMem: [ ");
    for (i = 0 ; i < 0x5 ; i++)
    {
        fprintf(fp, "$%02X ", cpu->memory_opcode_read(cpu->reg_PC + i));
    }
    fprintf(fp, "]\n");

    quick6502_getinstruction(cpu, true, cpu->reg_PC, instr, NULL);
    fprintf(fp, "## $%04X: %s\n", cpu->reg_PC, instr);
}

typedef enum InstructionNameTag
{
    n_ILG = 0, n_NOP,
    n_CLI, n_SEI, n_CLC, n_SEC, n_CLD, n_SED, n_CLV,
    n_LDA, n_LDX, n_LDY, n_STA, n_STX, n_STY,
    n_TXA, n_TAX, n_TAY, n_TYA, n_TSX, n_TXS,
    n_PHA, n_PLA, n_PHP, n_PLP,
    n_DEX, n_DEY, n_INX, n_INY, n_DEC, n_INC,
    n_JSR, n_RTS, n_JMP, n_BRK, n_RTI,
    n_BCC, n_BCS, n_BEQ, n_BNE, n_BPL, n_BVS, n_BVC, n_BMI,
    n_EOR, n_AND, n_BIT, n_ORA, n_ADC, n_SBC,
    n_ROL, n_ROR, n_ASL, n_LSR,
    n_CMP, n_CPX, n_CPY,
} InstructionNameTag;

char *InstructionName[] =
        {
                "ILLEGAL", "NOP",
                "CLI", "SEI", "CLC", "SEC", "CLD", "SED", "CLV",
                "LDA", "LDX", "LDY", "STA", "STX", "STY",
                "TXA", "TAX", "TAY", "TYA", "TSX", "TXS",
                "PHA", "PLA", "PHP", "PLP",
                "DEX", "DEY", "INX", "INY", "DEC", "INC",
                "JSR", "RTS", "JMP", "BRK", "RTI",
                "BCC", "BCS", "BEQ", "BNE", "BPL", "BVS", "BVC", "BVS",
                "EOR", "AND", "BIT", "ORA", "ADC", "SBC",
                "ROL", "ROR", "ASL", "LSR",
                "CMP", "CPX", "CPY",
        };

/**
 * Free the CPU 
 *
 * This function will free the CPU only if it's not currently used, it will
 * return 0 if everything goes well and !0 if the free is impossible
 */
int quick6502_free(quick6502_cpu *cpu)
{

    return 0;
}

/*******************************************************************************
 ***                          Here start real CPU logic                      ***
 *******************************************************************************/
static uint8_t CycleTable[256] =
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
    TRACEiE(("Illegal instruction $%02X", cpu->memory_opcode_read(cpu->reg_PC - 1)));
    //exit(-1);
}

/** 58 : CLI - CLear Interrupt **/
INSTRUCTION(CLInP)
{
    TRACEi(("CLI"));
    cpu->reg_P &= ~Q6502_I_FLAG;
}
/** 78 : SEI - SEt Interrupt **/
INSTRUCTION(SEInP)
{
    TRACEi(("SEI"));
    cpu->reg_P |= Q6502_I_FLAG;
}

/** 18 : CLC - CLear Carry **/
INSTRUCTION(CLCnP)
{
    TRACEi(("CLC"));
    cpu->reg_P &= ~Q6502_C_FLAG;
}
/** 38 : SEC - SEt Carry **/
INSTRUCTION(SECnP)
{
    TRACEi(("SEC"));
    cpu->reg_P |= Q6502_C_FLAG;
}

/** D8 : CLD - CLear Decimal **/
INSTRUCTION(CLDnP)
{
    TRACEi(("CLD"));
    cpu->reg_P &= ~Q6502_D_FLAG;
}
/** F8 : SED - SEt Decimal **/
INSTRUCTION(SEDnP)
{
    TRACEi(("SED"));
    cpu->reg_P |= Q6502_D_FLAG;
}
/** B8 : CLV - CLear oVerflow **/
INSTRUCTION(CLVnP)
{
    TRACEi(("CLV"));
    cpu->reg_P &= ~Q6502_V_FLAG;
}

/** EA : NOP - NO oPeration **/
INSTRUCTION(NOPnP)
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
    CROSS_CYCLE_UPDATE(cpu->memory_opcode_read(cpu->reg_PC - 1) + cpu->reg_Y);
}

/** AD: LDA $xxxx - LoaD to A **/
INSTRUCTION(LDAaB)
{
    TRACEi(("LDA $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    cpu->reg_A = MEMORY_READ_AB();
    NZ_FLAG_UPDATE(cpu->reg_A);
}

/** DD: LDA $xxxx,X - LoaD to A **/
INSTRUCTION(LDAaX)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);

    TRACEi(("LDA $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    cpu->reg_A = MEMORY_READ_AX();
    NZ_FLAG_UPDATE(cpu->reg_A);
    CROSS_CYCLE_UPDATE(op1 + cpu->reg_X);
}

/** D9: LDA $xxxx,Y - LoaD to A **/
INSTRUCTION(LDAaY)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);
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
    TRACEi(("STA $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    MEMORY_WRITE_AB(cpu->reg_A);
}

/** 9D: STA $xxxx,X - STore A **/
INSTRUCTION(STAaX)
{
    TRACEi(("STA $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    MEMORY_WRITE_AX(cpu->reg_A);
}

/** 99: STA $xxxx,Y - STore A **/
INSTRUCTION(STAaY)
{
    TRACEi(("STA $%02X%02X,Y", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
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
    TRACEi(("LDX $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    cpu->reg_X = MEMORY_READ_AB();
    NZ_FLAG_UPDATE(cpu->reg_X);
}

/** BE: LDX $xxxx,Y - LoaD to X **/
INSTRUCTION(LDXaY)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);
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
    TRACEi(("STX $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
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
    TRACEi(("LDY $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    cpu->reg_Y = MEMORY_READ_AB();
    NZ_FLAG_UPDATE(cpu->reg_Y);
}

/** BC: LDY $xxxx,X - LoaD to Y **/
INSTRUCTION(LDYaX)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);

    TRACEi(("LDY $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
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
    TRACEi(("STY $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    MEMORY_WRITE_AB(cpu->reg_Y);
}

/**** Register functions ****/

/** AA : TAX - Transfer A to X **/
INSTRUCTION(TAXnP)
{
    TRACEi(("TAX"));
    cpu->reg_X = cpu->reg_A;
    NZ_FLAG_UPDATE(cpu->reg_X);
}

/** 8A : TXA - Transfer X to A **/
INSTRUCTION(TXAnP)
{
    TRACEi(("TXA"));
    cpu->reg_A = cpu->reg_X;
    NZ_FLAG_UPDATE(cpu->reg_A);
}

/** A8 : TAY - Transfer A to Y **/
INSTRUCTION(TAYnP)
{
    TRACEi(("TAY"));
    cpu->reg_Y = cpu->reg_A;
    NZ_FLAG_UPDATE(cpu->reg_Y);
}

/** 98 : TYA - Transfer Y to A **/
INSTRUCTION(TYAnP)
{
    TRACEi(("TYA"));
    cpu->reg_A = cpu->reg_Y;
    NZ_FLAG_UPDATE(cpu->reg_A);
}

/* BA : TSX - Transfer S to X **/
INSTRUCTION(TSXnP)
{
    TRACEi(("TSX"));
    cpu->reg_X = cpu->reg_S;
    NZ_FLAG_UPDATE(cpu->reg_X);
}

/** 9A : TXS - Transfer X to S **/
INSTRUCTION(TXSnP)
{
    TRACEi(("TXS"));
    cpu->reg_S = cpu->reg_X;
}

/**** Simple register operation instructions ****/

/** CA : DEX - DEcrement X **/
INSTRUCTION(DEXnP)
{
    TRACEi(("DEX"));
    cpu->reg_X--;
    NZ_FLAG_UPDATE(cpu->reg_X);
}

/** 88 : DEY - DEcrement Y **/
INSTRUCTION(DEYnP)
{
    TRACEi(("DEY"));
    cpu->reg_Y--;
    NZ_FLAG_UPDATE(cpu->reg_Y);
}

/** E8 : INX - INcrement X **/
INSTRUCTION(INXnP)
{
    TRACEi(("INX"));
    cpu->reg_X++;
    NZ_FLAG_UPDATE(cpu->reg_X);
}

/** C8 : INY - INcrement Y **/
INSTRUCTION(INYnP)
{
    TRACEi(("INY"));
    cpu->reg_Y++;
    NZ_FLAG_UPDATE(cpu->reg_Y);
}

/**** Stack related instructions ****/

/** 48 : PHA - PusH A */
INSTRUCTION(PHAnP)
{
    TRACEi(("PHA"));
    PUSH_S(cpu->reg_A);
}

/** 68 : PLA - PuLl A */
INSTRUCTION(PLAnP)
{
    TRACEi(("PLA"));
    cpu->reg_A = POP_S();
    NZ_FLAG_UPDATE(cpu->reg_A);
}

/** 08 : PHP - PusH P */
INSTRUCTION(PHPnP)
{
    TRACEi(("PHP"));
    PUSH_S((cpu->reg_P | Q6502_R_FLAG | Q6502_B_FLAG));
}

/** 28 : PLP - PuLl P */
INSTRUCTION(PLPnP)
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
    TRACEi(("JSR $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    cpu->reg_PC++;
    PUSH_S((cpu->reg_PC >> 8) & 0xFF);
    PUSH_S((cpu->reg_PC) & 0xFF);
    cpu->reg_PC = ((cpu->memory_opcode_read(cpu->reg_PC - 1)) | (cpu->memory_opcode_read(cpu->reg_PC) << 8));
}

/** 60 : RTS - ReTurn from Subroutine */
INSTRUCTION(RTSnP)
{
    TRACEi(("RTS"));
    cpu->reg_PC = POP_S();
    cpu->reg_PC |= (POP_S() << 8);
    cpu->reg_PC++;
}

/** 4C : JMP $xxxx - JuMP unconditionally to $xxxx **/
INSTRUCTION(JMPaB)
{
    TRACEi(("JMP $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    cpu->reg_PC = cpu->memory_opcode_read(cpu->reg_PC) | (cpu->memory_opcode_read(cpu->reg_PC + 1) << 8);
}

/** 6C : JMP ($xxxx) - JuMP unconditionally to ($xxxx) **/
INSTRUCTION(JMPiD)
{
    TRACEi(("JMP ($%02X%02X)", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    cpu->reg_PC = cpu->memory_opcode_read(cpu->reg_PC) | (cpu->memory_opcode_read(cpu->reg_PC + 1) << 8);
    cpu->reg_PC = cpu->memory_read(cpu->reg_PC) |
                  (cpu->memory_read((cpu->reg_PC & 0xFF00) | ((cpu->reg_PC + 1) & 0x00FF)) << 8);
}

/** 00 : BRK - BReaK **/
INSTRUCTION(BRKnP)
{
    TRACEi(("BRK"));
    cpu->reg_PC++;
    PUSH_S((cpu->reg_PC >> 8) & 0xFF);
    PUSH_S((cpu->reg_PC) & 0xFF);
    PUSH_S(cpu->reg_P);
    cpu->reg_P = cpu->reg_P | Q6502_I_FLAG | Q6502_B_FLAG;

    cpu->reg_PC = (cpu->memory_read(Q6502_IRQ_LOW)) | (cpu->memory_read(Q6502_IRQ_HIGH) << 8);
}

/** 40 : RTI - ReTurn from Interruption **/
INSTRUCTION(RTInP)
{
    TRACEi(("RTI"));
    cpu->reg_P = POP_S();
    cpu->reg_PC = POP_S();
    cpu->reg_PC |= (POP_S() << 8);

    if (cpu->int_pending != 0)
    {
        quick6502_int(cpu, Q6502_IRQ_SIGNAL);
    }
}

/** 90 : BCC - Branch if Carry Clear **/
INSTRUCTION(BCCrE)
{
    TRACEi(("BCC $%04X", cpu->reg_PC + (signed char)cpu->memory_opcode_read(cpu->reg_PC) + 1));
    if (!(cpu->reg_P & Q6502_C_FLAG))
    {
        cpu->reg_PC += (signed char)cpu->memory_opcode_read(cpu->reg_PC) + 1;
        /* Need to set timing */

        /* +1 is same page */
        cpu->cycle_done += 1;
        /* +2 is another */
    }
    else
    {
        cpu->reg_PC++;
    }
}

/** B0 : BCS - Branch if Carry Set**/
INSTRUCTION(BCSrE)
{
    TRACEi(("BCS $%04X", cpu->reg_PC + (signed char)cpu->memory_opcode_read(cpu->reg_PC) + 1));
    if (cpu->reg_P & Q6502_C_FLAG)
    {
        cpu->reg_PC += (signed char)cpu->memory_opcode_read(cpu->reg_PC) + 1;
        /* Need to set timing */

        /* +1 is same page */
        cpu->cycle_done += 1;
        /* +2 is another */
    }
    else
    {
        cpu->reg_PC++;
    }
}

/** F0 : BEQ - Branch if Equal**/
INSTRUCTION(BEQrE)
{
    TRACEi(("BEQ $%04X", cpu->reg_PC + (signed char)cpu->memory_opcode_read(cpu->reg_PC) + 1));
    if (cpu->reg_P & Q6502_Z_FLAG)
    {
        cpu->reg_PC += (signed char)cpu->memory_opcode_read(cpu->reg_PC) + 1;
        /* Need to set timing */

        /* +1 is same page */
        cpu->cycle_done += 1;
        /* +2 is another */
    }
    else
    {
        cpu->reg_PC++;
    }
}

/** 30 : BMI - Branch if MInus**/
INSTRUCTION(BMIrE)
{
    TRACEi(("BMI $%04X", cpu->reg_PC + (signed char)cpu->memory_opcode_read(cpu->reg_PC) + 1));
    if (cpu->reg_P & Q6502_N_FLAG)
    {
        cpu->reg_PC += (signed char)cpu->memory_opcode_read(cpu->reg_PC) + 1;
        /* Need to set timing */

        /* +1 is same page */
        cpu->cycle_done += 1;
        /* +2 is another */
    }
    else
    {
        cpu->reg_PC++;
    }
}

/** D0 : Bxx - Branch if Not Equal**/
INSTRUCTION(BNErE)
{
    TRACEi(("BNE $%04X", cpu->reg_PC + (signed char)cpu->memory_opcode_read(cpu->reg_PC) + 1));
    if (!(cpu->reg_P & Q6502_Z_FLAG))
    {
        cpu->reg_PC += (signed char)cpu->memory_opcode_read(cpu->reg_PC) + 1;
        /* Need to set timing */

        /* +1 is same page */
        cpu->cycle_done += 1;
        /* +2 is another */
    }
    else
    {
        cpu->reg_PC++;
    }
}

/** 10 : BPL - Branch if PLus **/
INSTRUCTION(BPLrE)
{
    TRACEi(("BPL $%04X", cpu->reg_PC + (signed char)cpu->memory_opcode_read(cpu->reg_PC) + 1));
    if (!(cpu->reg_P & Q6502_N_FLAG))
    {
        cpu->reg_PC += (signed char)cpu->memory_opcode_read(cpu->reg_PC) + 1;
        /* Need to set timing */

        /* +1 is same page */
        cpu->cycle_done += 1;
        /* +2 is another */
    }
    else
    {
        cpu->reg_PC++;
    }
}

/** 50 : BVC - Branch if oVerflow Clear**/
INSTRUCTION(BVCrE)
{
    TRACEi(("BVC $%04X", cpu->reg_PC + (signed char)cpu->memory_opcode_read(cpu->reg_PC) + 1));
    if (!(cpu->reg_P & Q6502_V_FLAG))
    {
        cpu->reg_PC += (signed char)cpu->memory_opcode_read(cpu->reg_PC) + 1;
        /* Need to set timing */

        /* +1 is same page */
        cpu->cycle_done += 1;
        /* +2 is another */
    }
    else
    {
        cpu->reg_PC++;
    }
}

/** 70 : BVS - Branch if oVerflow Set**/
INSTRUCTION(BVSrE)
{
    TRACEi(("BVS $%04X", cpu->reg_PC + (signed char)cpu->memory_opcode_read(cpu->reg_PC) + 1));
    if (cpu->reg_P & Q6502_V_FLAG)
    {
        cpu->reg_PC += (signed char)cpu->memory_opcode_read(cpu->reg_PC) + 1;
        /* Need to set timing */

        /* +1 is same page */
        cpu->cycle_done += 1;
        /* +2 is another */
    }
    else
    {
        cpu->reg_PC++;
    }
}

/*** Mathematical functions ***/

/** 69 : ADC - ADd with Carry **/
INSTRUCTION(ADCiM)
{
    TRACEi(("ADC #$%02X", cpu->memory_opcode_read(cpu->reg_PC)));
    ADC_OPERATION(cpu->memory_opcode_read(cpu->reg_PC++));
}

/** 65 : ADC - ADd with Carry **/
INSTRUCTION(ADCzP)
{
    TRACEi(("ADC $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
    ADC_OPERATION(MEMORY_READ_ZP());
}

/** 75 : ADC - ADd with Carry **/
INSTRUCTION(ADCzX)
{
    TRACEi(("ADC $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
    ADC_OPERATION(MEMORY_READ_ZX());
}

/** 61 : ADC - ADd with Carry **/
INSTRUCTION(ADCiX)
{
    TRACEi(("ADC ($%02X,X)", cpu->memory_opcode_read(cpu->reg_PC)));
    ADC_OPERATION(MEMORY_READ_IX());
}

/** 71 : ADC - ADd with Carry **/
INSTRUCTION(ADCiY)
{
    TRACEi(("ADC ($%02X),Y", cpu->memory_opcode_read(cpu->reg_PC)));
    ADC_OPERATION(MEMORY_READ_IY());
    CROSS_CYCLE_UPDATE(cpu->memory_opcode_read(cpu->reg_PC - 1) + cpu->reg_Y);
}

/** 6D : ADC - ADd with Carry **/
INSTRUCTION(ADCaB)
{
    TRACEi(("ADC $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    ADC_OPERATION(MEMORY_READ_AB());
}

/** 7D : ADC - ADd with Carry **/
INSTRUCTION(ADCaX)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);

    TRACEi(("ADC $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    ADC_OPERATION(MEMORY_READ_AX());
    CROSS_CYCLE_UPDATE(op1 + cpu->reg_X);
}

/** 79 : ADC - ADd with Carry **/
INSTRUCTION(ADCaY)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);
    TRACEi(("ADC $%02X%02X,Y", op2, op1));
    ADC_OPERATION(MEMORY_READ_AY());
    CROSS_CYCLE_UPDATE(op1 + cpu->reg_Y);
}

/** E9 : SBC - SuBstract with Carry **/
INSTRUCTION(SBCiM)
{
    TRACEi(("SBC #$%02X", cpu->memory_opcode_read(cpu->reg_PC)));
    SBC_OPERATION(cpu->memory_opcode_read(cpu->reg_PC++));
}

/** E5 : SBC - SuBstract with Carry **/
INSTRUCTION(SBCzP)
{
    TRACEi(("SBC $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
    SBC_OPERATION(MEMORY_READ_ZP());
}

/** F5 : SBC - SuBstract with Carry **/
INSTRUCTION(SBCzX)
{
    TRACEi(("SBC $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
    SBC_OPERATION(MEMORY_READ_ZX());
}

/** E1 : SBC - SuBstract with Carry **/
INSTRUCTION(SBCiX)
{
    TRACEi(("SBC ($%02X,X)", cpu->memory_opcode_read(cpu->reg_PC)));
    SBC_OPERATION(MEMORY_READ_IX());
}

/** F1 : SBC - SuBstract with Carry **/
INSTRUCTION(SBCiY)
{
    TRACEi(("SBC ($%02X),Y", cpu->memory_opcode_read(cpu->reg_PC)));
    SBC_OPERATION(MEMORY_READ_IY());
    CROSS_CYCLE_UPDATE(cpu->memory_opcode_read(cpu->reg_PC - 1) + cpu->reg_Y);
}

/** ED : SBC - SuBstract with Carry **/
INSTRUCTION(SBCaB)
{
    TRACEi(("SBC $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    SBC_OPERATION(MEMORY_READ_AB());
}

/** FD : SBC - SuBstract with Carry **/
INSTRUCTION(SBCaX)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);

    TRACEi(("SBC $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    SBC_OPERATION(MEMORY_READ_AX());

    CROSS_CYCLE_UPDATE(op1 + cpu->reg_X);
}

/** F9 : SBC - SuBstract with Carry **/
INSTRUCTION(SBCaY)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);
    TRACEi(("SBC $%02X%02X,Y", op2, op1));
    SBC_OPERATION(MEMORY_READ_AY());
    CROSS_CYCLE_UPDATE(op1 + cpu->reg_Y);
}

/** C9 : CMP - CoMPare **/
INSTRUCTION(CMPiM)
{
    TRACEi(("CMP #$%02X", cpu->memory_opcode_read(cpu->reg_PC)));
    CMP_OPERATION(cpu->reg_A, cpu->memory_opcode_read(cpu->reg_PC++));
}

/** C5 : CMP - CoMPare **/
INSTRUCTION(CMPzP)
{
    TRACEi(("CMP $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
    CMP_OPERATION(cpu->reg_A, MEMORY_READ_ZP());
}

/** D5 : CMP - CoMPare **/
INSTRUCTION(CMPzX)
{
    TRACEi(("CMP $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
    CMP_OPERATION(cpu->reg_A, MEMORY_READ_ZX());
}

/** C1 : CMP - CoMPare **/
INSTRUCTION(CMPiX)
{
    TRACEi(("CMP ($%02X,X)", cpu->memory_opcode_read(cpu->reg_PC)));
    CMP_OPERATION(cpu->reg_A, MEMORY_READ_IX());
}

/** D1 : CMP - CoMPare **/
INSTRUCTION(CMPiY)
{
    TRACEi(("CMP ($%02X),Y", cpu->memory_opcode_read(cpu->reg_PC)));
    CMP_OPERATION(cpu->reg_A, MEMORY_READ_IY());
    CROSS_CYCLE_UPDATE(cpu->memory_opcode_read(cpu->reg_PC - 1) + cpu->reg_Y);
}

/** CD : CMP - CoMPare **/
INSTRUCTION(CMPaB)
{
    TRACEi(("CMP $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    CMP_OPERATION(cpu->reg_A, MEMORY_READ_AB());
}

/** DD : CMP - CoMPare **/
INSTRUCTION(CMPaX)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);

    TRACEi(("CMP $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    CMP_OPERATION(cpu->reg_A, MEMORY_READ_AX());
    CROSS_CYCLE_UPDATE(op1 + cpu->reg_X);
}

/** D9 : CMP - CoMPare **/
INSTRUCTION(CMPaY)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);
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
    TRACEi(("CPX $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
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
    TRACEi(("CPY $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    CMP_OPERATION(cpu->reg_Y, MEMORY_READ_AB());
}

/** 09 : ORA - OR with A **/
INSTRUCTION(ORAiM)
{
    TRACEi(("ORA #$%02X", cpu->memory_opcode_read(cpu->reg_PC)));
    ORA_OPERATION(cpu->memory_opcode_read(cpu->reg_PC++));
}

/** 405 : ORA - OR with A **/
INSTRUCTION(ORAzP)
{
    TRACEi(("ORA $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
    ORA_OPERATION(cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC++)));
}

/** 15 : ORA - OR with A **/
INSTRUCTION(ORAzX)
{
    TRACEi(("ORA $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
    ORA_OPERATION(MEMORY_READ_ZX());
}

/** 01 : ORA - OR with A **/
INSTRUCTION(ORAiX)
{
    TRACEi(("ORA ($%02X,X)", cpu->memory_opcode_read(cpu->reg_PC)));
    ORA_OPERATION(MEMORY_READ_IX());
}

/** 11 : ORA - OR with A **/
INSTRUCTION(ORAiY)
{
    TRACEi(("ORA ($%02X),Y", cpu->memory_opcode_read(cpu->reg_PC)));
    ORA_OPERATION(MEMORY_READ_IY());
    CROSS_CYCLE_UPDATE(cpu->memory_opcode_read(cpu->reg_PC - 1) + cpu->reg_Y);
}

/** 0D : ORA - OR with A **/
INSTRUCTION(ORAaB)
{
    TRACEi(("ORA $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    ORA_OPERATION(MEMORY_READ_AB());
}

/** 1D : ORA - OR with A **/
INSTRUCTION(ORAaX)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);

    TRACEi(("ORA $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    ORA_OPERATION(MEMORY_READ_AX());
    CROSS_CYCLE_UPDATE(op1 + cpu->reg_X);
}

/** 19 : ORA - OR with A **/
INSTRUCTION(ORAaY)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);

    TRACEi(("ORA $%02X%02X,Y", op2, op1));
    ORA_OPERATION(MEMORY_READ_AY());
    CROSS_CYCLE_UPDATE(op1 + cpu->reg_Y);
}

/** 49 : EOR - Exclusive OR **/
INSTRUCTION(EORiM)
{
    TRACEi(("EOR #$%02X", cpu->memory_opcode_read(cpu->reg_PC)));
    EOR_OPERATION(cpu->memory_opcode_read(cpu->reg_PC++));
}

/** 45 : EOR - Exclusive OR **/
INSTRUCTION(EORzP)
{
    TRACEi(("EOR $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
    EOR_OPERATION(cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC++)));
}

/** 55 : EOR - Exclusive OR **/
INSTRUCTION(EORzX)
{
    TRACEi(("EOR $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
    EOR_OPERATION(MEMORY_READ_ZX());
}

/** 41 : EOR - Exclusive OR **/
INSTRUCTION(EORiX)
{
    TRACEi(("EOR ($%02X,X)", cpu->memory_opcode_read(cpu->reg_PC)));
    EOR_OPERATION(MEMORY_READ_IX());
}

/** 51 : EOR - Exclusive OR **/
INSTRUCTION(EORiY)
{
    TRACEi(("EOR ($%02X),Y", cpu->memory_opcode_read(cpu->reg_PC)));
    EOR_OPERATION(MEMORY_READ_IY());

    CROSS_CYCLE_UPDATE(cpu->memory_opcode_read(cpu->reg_PC - 1) + cpu->reg_Y);
}

/** 4D : EOR - Exclusive OR **/
INSTRUCTION(EORaB)
{
    TRACEi(("EOR $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    EOR_OPERATION(MEMORY_READ_AB());
}

/** 5D : EOR - Exclusive OR **/
INSTRUCTION(EORaX)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);

    TRACEi(("EOR $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    EOR_OPERATION(MEMORY_READ_AX());
    CROSS_CYCLE_UPDATE(op1 + cpu->reg_X);
}

/** 59 : EOR - Exclusive OR **/
INSTRUCTION(EORaY)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);
    TRACEi(("EOR $%02X%02X,Y", op2, op1));
    EOR_OPERATION(MEMORY_READ_AY());
    CROSS_CYCLE_UPDATE(op1 + cpu->reg_Y);
}

/** 29 : AND - Logical AND **/
INSTRUCTION(ANDiM)
{
    TRACEi(("AND #$%02X", cpu->memory_opcode_read(cpu->reg_PC)));
    AND_OPERATION(cpu->memory_opcode_read(cpu->reg_PC++));
}

/** 25 : AND - Logical AND **/
INSTRUCTION(ANDzP)
{
    TRACEi(("AND $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
    AND_OPERATION(cpu->memory_page0_read(cpu->memory_opcode_read(cpu->reg_PC++)));
}

/** 35 : AND - Logical AND **/
INSTRUCTION(ANDzX)
{
    TRACEi(("AND $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
    AND_OPERATION(MEMORY_READ_ZX());
}

/** 21 : AND - Logical AND **/
INSTRUCTION(ANDiX)
{
    TRACEi(("AND ($%02X,X)", cpu->memory_opcode_read(cpu->reg_PC)));
    AND_OPERATION(MEMORY_READ_IX());
}

/** 31 : AND - Logical AND **/
INSTRUCTION(ANDiY)
{
    TRACEi(("AND ($%02X),Y", cpu->memory_opcode_read(cpu->reg_PC)));
    AND_OPERATION(MEMORY_READ_IY());

    CROSS_CYCLE_UPDATE(cpu->memory_opcode_read(cpu->reg_PC - 1) + cpu->reg_Y);
}

/** 2D : AND - Logical AND **/
INSTRUCTION(ANDaB)
{
    TRACEi(("AND $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    AND_OPERATION(MEMORY_READ_AB());
}

/** 3D : AND - Logical AND **/
INSTRUCTION(ANDaX)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);

    TRACEi(("AND $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    AND_OPERATION(MEMORY_READ_AX());

    CROSS_CYCLE_UPDATE(op1 + cpu->reg_X);
}

/** 39 : AND - Logical AND **/
INSTRUCTION(ANDaY)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);
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
    TRACEi(("BIT $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    BIT_OPERATION(MEMORY_READ_AB());
}

/** 2A : ROL A **/
INSTRUCTION(ROLnP)
{
    TRACEi(("ROL A"));
    ROL_OPERATION(cpu->reg_A);
}

/** 6A : ROR A **/
INSTRUCTION(RORnP)
{
    TRACEi(("ROR A"));
    ROR_OPERATION(cpu->reg_A);
}

/** 0A : ASL A **/
INSTRUCTION(ASLnP)
{
    TRACEi(("ASL A"));
    ASL_OPERATION(cpu->reg_A);
}

/** 4A : LSR A **/
INSTRUCTION(LSRnP)
{
    TRACEi(("LSR A"));
    LSR_OPERATION(cpu->reg_A);
}

/** 2E : ROL **/
INSTRUCTION(ROLaB)
{
    TRACEi(("ROL $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_AB();
    cpu->reg_PC -= 2;
    ROL_OPERATION(val);
    MEMORY_WRITE_AB(val);
}

/** 26 : ROL **/
INSTRUCTION(ROLzP)
{
    TRACEi(("ROL $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_ZP();
    cpu->reg_PC -= 1;
    ROL_OPERATION(val);
    MEMORY_WRITE_ZP(val);
}

/** 3E : ROL **/
INSTRUCTION(ROLaX)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);

    TRACEi(("ROL $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_AX();
    cpu->reg_PC -= 2;
    ROL_OPERATION(val);
    MEMORY_WRITE_AX(val);
}

/** 36 : ROL **/
INSTRUCTION(ROLzX)
{
    TRACEi(("ROL $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_ZX();
    cpu->reg_PC -= 1;
    ROL_OPERATION(val);
    MEMORY_WRITE_ZX(val);
}

/** 6E : ROR **/
INSTRUCTION(RORaB)
{
    TRACEi(("ROR $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_AB();
    cpu->reg_PC -= 2;
    ROR_OPERATION(val);
    MEMORY_WRITE_AB(val);
}

/** 66 : ROR **/
INSTRUCTION(RORzP)
{
    TRACEi(("ROR $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_ZP();
    cpu->reg_PC -= 1;
    ROR_OPERATION(val);
    MEMORY_WRITE_ZP(val);
}

/** 7E : ROR **/
INSTRUCTION(RORaX)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);

    TRACEi(("ROR $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_AX();
    cpu->reg_PC -= 2;
    ROR_OPERATION(val);
    MEMORY_WRITE_AX(val);
}

/** 76 : ROR **/
INSTRUCTION(RORzX)
{
    TRACEi(("ROR $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_ZX();
    cpu->reg_PC -= 1;
    ROR_OPERATION(val);
    MEMORY_WRITE_ZX(val);
}

/** 0E : ASL **/
INSTRUCTION(ASLaB)
{
    TRACEi(("ASL $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_AB();
    cpu->reg_PC -= 2;
    ASL_OPERATION(val);
    MEMORY_WRITE_AB(val);
}

/** 06 : ASL **/
INSTRUCTION(ASLzP)
{
    TRACEi(("ASL $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_ZP();
    cpu->reg_PC -= 1;
    ASL_OPERATION(val);
    MEMORY_WRITE_ZP(val);
}

/** 1E : ASL **/
INSTRUCTION(ASLaX)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);

    TRACEi(("ASL $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_AX();
    cpu->reg_PC -= 2;
    ASL_OPERATION(val);
    MEMORY_WRITE_AX(val);
}

/** 16 : ASL **/
INSTRUCTION(ASLzX)
{
    TRACEi(("ASL $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_ZX();
    cpu->reg_PC -= 1;
    ASL_OPERATION(val);
    MEMORY_WRITE_ZX(val);
}

/** 4E : LSR **/
INSTRUCTION(LSRaB)
{
    TRACEi(("LSR $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_AB();
    cpu->reg_PC -= 2;
    LSR_OPERATION(val);
    MEMORY_WRITE_AB(val);
}

/** 46 : LSR **/
INSTRUCTION(LSRzP)
{
    TRACEi(("LSR $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_ZP();
    cpu->reg_PC -= 1;
    LSR_OPERATION(val);
    MEMORY_WRITE_ZP(val);
}

/** 5E : LSR **/
INSTRUCTION(LSRaX)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);

    TRACEi(("LSR $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_AX();
    cpu->reg_PC -= 2;
    LSR_OPERATION(val);
    MEMORY_WRITE_AX(val);
}

/** 56 : LSR **/
INSTRUCTION(LSRzX)
{
    TRACEi(("LSR $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_ZX();
    cpu->reg_PC -= 1;
    LSR_OPERATION(val);
    MEMORY_WRITE_ZX(val);
}

/** CE : DEC **/
INSTRUCTION(DECaB)
{
    TRACEi(("DEC $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_AB();
    cpu->reg_PC -= 2;
    MEMORY_WRITE_AB(--val);
    NZ_FLAG_UPDATE(val);
}

/** C6 : DEC **/
INSTRUCTION(DECzP)
{
    TRACEi(("DEC $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_ZP();
    cpu->reg_PC -= 1;
    MEMORY_WRITE_ZP(--val);
    NZ_FLAG_UPDATE(val);
}

/** DE : DEC **/
INSTRUCTION(DECaX)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);

    TRACEi(("DEC $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_AX();
    cpu->reg_PC -= 2;
    MEMORY_WRITE_AX(--val);
    NZ_FLAG_UPDATE(val);
}

/** D6 : DEC **/
INSTRUCTION(DECzX)
{
    TRACEi(("DEC $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_ZX();
    cpu->reg_PC -= 1;
    MEMORY_WRITE_ZX(--val);
    NZ_FLAG_UPDATE(val);
}

/** EE : INC **/
INSTRUCTION(INCaB)
{
    TRACEi(("INC $%02X%02X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_AB();
    cpu->reg_PC -= 2;
    MEMORY_WRITE_AB(++val);
    NZ_FLAG_UPDATE(val);
}

/** E6 : INC **/
INSTRUCTION(INCzP)
{
    TRACEi(("INC $%02X", cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_ZP();
    cpu->reg_PC -= 1;
    MEMORY_WRITE_ZP(++val);
    NZ_FLAG_UPDATE(val);
}

/** FE : INC **/
INSTRUCTION(INCaX)
{
    register uint8_t op1 = cpu->memory_opcode_read(cpu->reg_PC++);
    register uint8_t op2 = cpu->memory_opcode_read(cpu->reg_PC++);

    TRACEi(("INC $%02X%02X,X", cpu->memory_opcode_read(cpu->reg_PC + 1), cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_AX();
    cpu->reg_PC -= 2;
    MEMORY_WRITE_AX(++val);
    NZ_FLAG_UPDATE(val);
}

/** F6 : INC **/
INSTRUCTION(INCzX)
{
    TRACEi(("INC $%02X,X", cpu->memory_opcode_read(cpu->reg_PC)));
    uint8_t val = MEMORY_READ_ZX();
    cpu->reg_PC -= 1;
    MEMORY_WRITE_ZX(++val);
    NZ_FLAG_UPDATE(val);
}

/* iM: Immediate
 * iX: Indirect by X
 * iY: Indirect by Y
 * zP: Zero Page
 * zX: Zero Page Index by X
 * zY: Zero Page Index by Y
 * iD: Indirect Double
 * aB: Absolute
 * aX: Absolute by X
 * aY: Absolute by Y 
 */
static InstructionFunction InstructionTable[256] =
        {
                /*         00       01       02       03       04       05       06       07       08       09       0A       0B       0C       0D       0E       0F */
                /* 00 */ I_BRKnP, I_ORAiX, I_ILLEG, I_ILLEG, I_ILLEG, I_ORAzP, I_ASLzP, I_ILLEG, I_PHPnP, I_ORAiM,
                         I_ASLnP, I_ILLEG, I_ILLEG, I_ORAaB, I_ASLaB, I_ILLEG,
                /* 10 */ I_BPLrE, I_ORAiY, I_ILLEG, I_ILLEG, I_ILLEG, I_ORAzX, I_ASLzX, I_ILLEG, I_CLCnP, I_ORAaY,
                         I_ILLEG, I_ILLEG, I_ILLEG, I_ORAaX, I_ASLaX, I_ILLEG,
                /* 20 */ I_JSRaB, I_ANDiX, I_ILLEG, I_ILLEG, I_BITzP, I_ANDzP, I_ROLzP, I_ILLEG, I_PLPnP, I_ANDiM,
                         I_ROLnP, I_ILLEG, I_BITaB, I_ANDaB, I_ROLaB, I_ILLEG,
                /* 30 */ I_BMIrE, I_ANDiY, I_ILLEG, I_ILLEG, I_ILLEG, I_ANDzX, I_ROLzX, I_ILLEG, I_SECnP, I_ANDaY,
                         I_ILLEG, I_ILLEG, I_ILLEG, I_ANDaX, I_ROLaX, I_ILLEG,
                /* 40 */ I_RTInP, I_EORiX, I_ILLEG, I_ILLEG, I_ILLEG, I_EORzP, I_LSRzP, I_ILLEG, I_PHAnP, I_EORiM,
                         I_LSRnP, I_ILLEG, I_JMPaB, I_EORaB, I_LSRaB, I_ILLEG,
                /* 50 */ I_BVCrE, I_EORiY, I_ILLEG, I_ILLEG, I_ILLEG, I_EORzX, I_LSRzX, I_ILLEG, I_CLInP, I_EORaY,
                         I_ILLEG, I_ILLEG, I_ILLEG, I_EORaX, I_LSRaX, I_ILLEG,
                /* 60 */ I_RTSnP, I_ADCiX, I_ILLEG, I_ILLEG, I_ILLEG, I_ADCzP, I_RORzP, I_ILLEG, I_PLAnP, I_ADCiM,
                         I_RORnP, I_ILLEG, I_JMPiD, I_ADCaB, I_RORaB, I_ILLEG,
                /* 70 */ I_BVSrE, I_ADCiY, I_ILLEG, I_ILLEG, I_ILLEG, I_ADCzX, I_RORzX, I_ILLEG, I_SEInP, I_ADCaY,
                         I_ILLEG, I_ILLEG, I_ILLEG, I_ADCaX, I_RORaX, I_ILLEG,
                /* 80 */ I_ILLEG, I_STAiX, I_ILLEG, I_ILLEG, I_STYzP, I_STAzP, I_STXzP, I_ILLEG, I_DEYnP, I_ILLEG,
                         I_TXAnP, I_ILLEG, I_STYaB, I_STAaB, I_STXaB, I_ILLEG,
                /* 90 */ I_BCCrE, I_STAiY, I_ILLEG, I_ILLEG, I_STYzX, I_STAzX, I_STXzY, I_ILLEG, I_TYAnP, I_STAaY,
                         I_TXSnP, I_ILLEG, I_ILLEG, I_STAaX, I_ILLEG, I_ILLEG,
                /* A0 */ I_LDYiM, I_LDAiX, I_LDXiM, I_ILLEG, I_LDYzP, I_LDAzP, I_LDXzP, I_ILLEG, I_TAYnP, I_LDAiM,
                         I_TAXnP, I_ILLEG, I_LDYaB, I_LDAaB, I_LDXaB, I_ILLEG,
                /* B0 */ I_BCSrE, I_LDAiY, I_ILLEG, I_ILLEG, I_LDYzX, I_LDAzX, I_LDXzY, I_ILLEG, I_CLVnP, I_LDAaY,
                         I_TSXnP, I_ILLEG, I_LDYaX, I_LDAaX, I_LDXaY, I_ILLEG,
                /* C0 */ I_CPYiM, I_CMPiX, I_ILLEG, I_ILLEG, I_CPYzP, I_CMPzP, I_DECzP, I_ILLEG, I_INYnP, I_CMPiM,
                         I_DEXnP, I_ILLEG, I_CPYaB, I_CMPaB, I_DECaB, I_ILLEG,
                /* D0 */ I_BNErE, I_CMPiY, I_ILLEG, I_ILLEG, I_ILLEG, I_CMPzX, I_DECzX, I_ILLEG, I_CLDnP, I_CMPaY,
                         I_ILLEG, I_ILLEG, I_ILLEG, I_CMPaX, I_DECaX, I_ILLEG,
                /* E0 */ I_CPXiM, I_SBCiX, I_ILLEG, I_ILLEG, I_CPXzP, I_SBCzP, I_INCzP, I_ILLEG, I_INXnP, I_SBCiM,
                         I_NOPnP, I_ILLEG, I_CPXaB, I_SBCaB, I_INCaB, I_ILLEG,
                /* F0 */ I_BEQrE, I_SBCiY, I_ILLEG, I_ILLEG, I_ILLEG, I_SBCzX, I_INCzX, I_ILLEG, I_SEDnP, I_SBCaY,
                         I_ILLEG, I_ILLEG, I_ILLEG, I_SBCaX, I_INCaX, I_ILLEG
                /*         00       01       02       03       04       05       06       07       08       09       0A       0B       0C       0D       0E       0F */
        };

#if 1

typedef enum InstructionType
{
    t_IMM = 0, t_IDX, t_IDY, t_ABS,
    t_REL, t_ZEP, t_ZPX, t_ZPY,
    t_ABX, t_ABY, t_IND, t_NOP,
} InstructionType;

static InstructionType InstructionTypeTable[256] =
        {
/*       00     01     02     03     04     05     06     07     08     09     0A     0B     0C     0D     0E     0F */
/* 00 */ t_NOP, t_IDX, t_IMM, t_IMM, t_IMM, t_ZEP, t_ZEP, t_IMM, t_NOP, t_IMM, t_NOP, t_IMM, t_IMM, t_ABS, t_ABS, t_IMM,
/* 10 */ t_REL, t_IDY, t_IMM, t_IMM, t_IMM, t_ZPX, t_ZPX, t_IMM, t_NOP, t_ABY, t_IMM, t_IMM, t_IMM, t_ABX, t_ABX, t_IMM,
/* 20 */ t_ABS, t_IDX, t_IMM, t_IMM, t_ZEP, t_ZEP, t_ZEP, t_IMM, t_NOP, t_IMM, t_NOP, t_IMM, t_ABS, t_ABS, t_ABS, t_IMM,
/* 30 */ t_REL, t_IDY, t_IMM, t_IMM, t_IMM, t_ZPX, t_ZPX, t_IMM, t_NOP, t_ABY, t_IMM, t_IMM, t_IMM, t_ABX, t_ABX, t_IMM,
/* 40 */ t_NOP, t_IDX, t_IMM, t_IMM, t_IMM, t_ZEP, t_ZEP, t_IMM, t_NOP, t_IMM, t_NOP, t_IMM, t_ABS, t_ABS, t_ABS, t_IMM,
/* 50 */ t_REL, t_IDY, t_IMM, t_IMM, t_IMM, t_ZPX, t_ZPX, t_IMM, t_NOP, t_ABY, t_IMM, t_IMM, t_IMM, t_ABX, t_ABX, t_IMM,
/* 60 */ t_NOP, t_IDX, t_IMM, t_IMM, t_IMM, t_ZEP, t_ZEP, t_IMM, t_NOP, t_IMM, t_NOP, t_IMM, t_IND, t_ABS, t_ABS, t_IMM,
/* 70 */ t_REL, t_IDY, t_IMM, t_IMM, t_IMM, t_ZPX, t_ZPX, t_IMM, t_NOP, t_ABY, t_IMM, t_IMM, t_IMM, t_ABX, t_ABX, t_IMM,
/* 80 */ t_IMM, t_IDX, t_IMM, t_IMM, t_ZEP, t_ZEP, t_ZEP, t_IMM, t_NOP, t_IMM, t_NOP, t_IMM, t_ABS, t_ABS, t_ABS, t_IMM,
/* 90 */ t_REL, t_IDY, t_IMM, t_IMM, t_ZPX, t_ZPX, t_ZPY, t_IMM, t_NOP, t_ABY, t_NOP, t_IMM, t_IMM, t_ABX, t_IMM, t_IMM,
/* A0 */ t_IMM, t_IDX, t_IMM, t_IMM, t_ZEP, t_ZEP, t_ZEP, t_IMM, t_NOP, t_IMM, t_NOP, t_IMM, t_ABS, t_ABS, t_ABS, t_IMM,
/* B0 */ t_REL, t_IDY, t_IMM, t_IMM, t_ZPX, t_ZPX, t_ZPY, t_IMM, t_NOP, t_ABY, t_NOP, t_IMM, t_ABX, t_ABX, t_ABY, t_IMM,
/* C0 */ t_IMM, t_IDX, t_IMM, t_IMM, t_ZEP, t_ZEP, t_ZEP, t_IMM, t_NOP, t_IMM, t_NOP, t_IMM, t_ABS, t_ABS, t_ABS, t_IMM,
/* D0 */ t_REL, t_IDY, t_IMM, t_IMM, t_IMM, t_ZPX, t_ZPX, t_IMM, t_NOP, t_ABY, t_IMM, t_IMM, t_IMM, t_ABX, t_ABX, t_IMM,
/* E0 */ t_IMM, t_IDX, t_IMM, t_IMM, t_ZEP, t_ZEP, t_ZEP, t_IMM, t_NOP, t_IMM, t_IMM, t_IMM, t_ABS, t_ABS, t_ABS, t_IMM,
/* F0 */ t_REL, t_IDY, t_IMM, t_IMM, t_IMM, t_ZPX, t_ZPX, t_IMM, t_NOP, t_ABY, t_IMM, t_IMM, t_IMM, t_ABX, t_ABX, t_IMM
/*       00     01     02     03     04     05     06     07     08     09     0A     0B     0C     0D     0E     0F */
        };

static InstructionNameTag InstructionNameTable[256] =
        {
/*       00     01     02     03     04     05     06     07     08     09     0A     0B     0C     0D     0E     0F */
/* 00 */ n_BRK, n_ORA, n_ILG, n_ILG, n_ILG, n_ORA, n_ASL, n_ILG, n_PHP, n_ORA, n_ASL, n_ILG, n_ILG, n_ORA, n_ASL, n_ILG,
/* 10 */ n_BPL, n_ORA, n_ILG, n_ILG, n_ILG, n_ORA, n_ASL, n_ILG, n_CLC, n_ORA, n_ILG, n_ILG, n_ILG, n_ORA, n_ASL, n_ILG,
/* 20 */ n_JSR, n_AND, n_ILG, n_ILG, n_BIT, n_AND, n_ROL, n_ILG, n_PLP, n_AND, n_ROL, n_ILG, n_BIT, n_AND, n_ROL, n_ILG,
/* 30 */ n_BMI, n_AND, n_ILG, n_ILG, n_ILG, n_AND, n_ROL, n_ILG, n_SEC, n_AND, n_ILG, n_ILG, n_ILG, n_AND, n_ROL, n_ILG,
/* 40 */ n_RTI, n_EOR, n_ILG, n_ILG, n_ILG, n_EOR, n_LSR, n_ILG, n_PHA, n_EOR, n_LSR, n_ILG, n_JMP, n_EOR, n_LSR, n_ILG,
/* 50 */ n_BVC, n_EOR, n_ILG, n_ILG, n_ILG, n_EOR, n_LSR, n_ILG, n_CLI, n_EOR, n_ILG, n_ILG, n_ILG, n_EOR, n_LSR, n_ILG,
/* 60 */ n_RTS, n_ADC, n_ILG, n_ILG, n_ILG, n_ADC, n_ROR, n_ILG, n_PLA, n_ADC, n_ROR, n_ILG, n_JMP, n_ADC, n_ROR, n_ILG,
/* 70 */ n_BVS, n_ADC, n_ILG, n_ILG, n_ILG, n_ADC, n_ROR, n_ILG, n_SEI, n_ADC, n_ILG, n_ILG, n_ILG, n_ADC, n_ROR, n_ILG,
/* 80 */ n_ILG, n_STA, n_ILG, n_ILG, n_STY, n_STA, n_STX, n_ILG, n_DEY, n_ILG, n_TXA, n_ILG, n_STY, n_STA, n_STX, n_ILG,
/* 90 */ n_BCC, n_STA, n_ILG, n_ILG, n_STY, n_STA, n_STX, n_ILG, n_TYA, n_STA, n_TXS, n_ILG, n_ILG, n_STA, n_ILG, n_ILG,
/* A0 */ n_LDY, n_LDA, n_LDX, n_ILG, n_LDY, n_LDA, n_LDX, n_ILG, n_TAY, n_LDA, n_TAX, n_ILG, n_LDY, n_LDA, n_LDX, n_ILG,
/* B0 */ n_BCS, n_LDA, n_ILG, n_ILG, n_LDY, n_LDA, n_LDX, n_ILG, n_CLV, n_LDA, n_TSX, n_ILG, n_LDY, n_LDA, n_LDX, n_ILG,
/* C0 */ n_CPY, n_CMP, n_ILG, n_ILG, n_CPY, n_CMP, n_DEC, n_ILG, n_INY, n_CMP, n_DEX, n_ILG, n_CPY, n_CMP, n_DEC, n_ILG,
/* D0 */ n_BNE, n_CMP, n_ILG, n_ILG, n_ILG, n_CMP, n_DEC, n_ILG, n_CLD, n_CMP, n_ILG, n_ILG, n_ILG, n_CMP, n_DEC, n_ILG,
/* E0 */ n_CPX, n_SBC, n_ILG, n_ILG, n_CPX, n_SBC, n_INC, n_ILG, n_INX, n_SBC, n_NOP, n_ILG, n_CPX, n_SBC, n_INC, n_ILG,
/* F0 */ n_BEQ, n_SBC, n_ILG, n_ILG, n_ILG, n_SBC, n_INC, n_ILG, n_SED, n_SBC, n_ILG, n_ILG, n_ILG, n_SBC, n_INC, n_ILG,
/*       00     01     02     03     04     05     06     07     08     09     0A     0B     0C     0D     0E     0F */
        };

/** Get current instruction name at specified address and put it into buffer */
int quick6502_getinstruction(quick6502_cpu *cpu, char interpret,
                             uint16_t addr, char *buffer, int *strlength)
{
    int len = 0, curlen;
    int readuint8_t = 1;
    char *str = buffer;

    uint8_t opcode = cpu->memory_opcode_read(addr);

    uint8_t value_u8;
    uint16_t value_u16;

    curlen = sprintf(str, "%s", InstructionName[InstructionNameTable[opcode]]);
    str += curlen;
    len += curlen;

    switch (InstructionTypeTable[opcode])
    {
    default: /* Nothing to do */
    case t_NOP:
        break;

    case t_IMM:
        curlen = sprintf(str, " #$%02X", cpu->memory_opcode_read(addr + 1));
        str += curlen;
        len += curlen;

        /* Nothing to interpret.. Really */

        readuint8_t += 2;

        break;

    case t_IDX:
        curlen = sprintf(str, " ($%02X, X)", cpu->memory_opcode_read(addr + 1));
        str += curlen;
        len += curlen;

        if (interpret)
        {
            value_u8 = cpu->memory_opcode_read(addr + 1);
            value_u16 = value_u8 + cpu->reg_X;

            curlen = sprintf(str, " ; ($%02X + $%02X) -> ($%02X) -> $%02X%02X",
                             value_u8, cpu->reg_X,
                             value_u16 & 0xFF,
                             cpu->memory_page0_read(value_u16),
                             cpu->memory_page0_read(value_u16 + 1));
            str += curlen;
            len += curlen;
        }

        readuint8_t += 1;
        break;

    case t_IDY:
        curlen = sprintf(str, " ($%02X), Y", cpu->memory_opcode_read(addr + 1));
        str += curlen;
        len += curlen;

        if (interpret)
        {
            value_u8 = cpu->memory_opcode_read(addr + 1);
            value_u16 = (cpu->memory_page0_read(value_u8 + 1) << 8) |
                        (cpu->memory_page0_read(value_u8));

            curlen = sprintf(str, " ; ($%02X) + $%02X -> $%04X + $%02X -> $%04X",
                             value_u8, cpu->reg_Y,
                             value_u16, cpu->reg_Y,
                             value_u16 + cpu->reg_Y
            );
            str += curlen;
            len += curlen;
        }

        readuint8_t += 1;
        break;

    case t_ABS:
        curlen = sprintf(str, " $%02X%02X", cpu->memory_opcode_read(addr + 2),
                         cpu->memory_opcode_read(addr + 1));
        str += curlen;
        len += curlen;

        /* Nothing to interpret.. Really */

        readuint8_t += 2;
        break;

    case t_REL:
        value_u16 = 2 + addr + (signed char)cpu->memory_opcode_read(addr + 1);
        curlen = sprintf(str, " $%04X", value_u16);
        str += curlen;
        len += curlen;

        /* Nothing to interpret.. Really */

        readuint8_t += 1;
        break;

    case t_ZEP:
        curlen = sprintf(str, " $%02X", cpu->memory_opcode_read(addr + 1));
        str += curlen;
        len += curlen;

        /* Nothing to interpret.. Really */

        readuint8_t += 1;
        break;

    case t_ZPX:
        curlen = sprintf(str, " $%02X, X", cpu->memory_opcode_read(addr + 1));
        str += curlen;
        len += curlen;

        if (interpret)
        {
            curlen = sprintf(str, " ; $%02X + $%02x -> $%02X",
                             cpu->memory_opcode_read(addr + 1), cpu->reg_X,
                             (cpu->memory_opcode_read(addr + 1) + cpu->reg_X) & 0xFF);
            str += curlen;
            len += curlen;
        }

        readuint8_t += 1;
        break;

    case t_ZPY:
        curlen = sprintf(str, " $%02X, Y", cpu->memory_opcode_read(addr + 1));
        str += curlen;
        len += curlen;

        if (interpret)
        {
            curlen = sprintf(str, " ; $%02X + $%02x -> $%02X",
                             cpu->memory_opcode_read(addr + 1), cpu->reg_Y,
                             (cpu->memory_opcode_read(addr + 1) + cpu->reg_Y) & 0xFF);
            str += curlen;
            len += curlen;
        }

        readuint8_t += 1;
        break;

    case t_ABX:
        curlen = sprintf(str, " $%02X%02X, X", cpu->memory_opcode_read(addr + 2),
                         cpu->memory_opcode_read(addr + 1));
        str += curlen;
        len += curlen;

        if (interpret)
        {
            value_u16 = (cpu->memory_opcode_read(addr + 2) << 8) |
                        cpu->memory_opcode_read(addr + 1);
            curlen = sprintf(str, " ; $%04X + $%02X -> $%04X", value_u16,
                             cpu->reg_X, value_u16 + cpu->reg_X);
            str += curlen;
            len += curlen;
        }

        readuint8_t += 2;
        break;

    case t_ABY:
        curlen = sprintf(str, " $%02X%02X, Y", cpu->memory_opcode_read(addr + 2),
                         cpu->memory_opcode_read(addr + 1));
        str += curlen;
        len += curlen;

        if (interpret)
        {
            value_u16 = (cpu->memory_opcode_read(addr + 2) << 8) |
                        cpu->memory_opcode_read(addr + 1);
            curlen = sprintf(str, " ; $%04X + $%02X -> $%04X", value_u16,
                             cpu->reg_Y, value_u16 + cpu->reg_Y);
            str += curlen;
            len += curlen;
        }

        readuint8_t += 2;
        break;

    case t_IND:
        curlen = sprintf(str, " ($%02X%02X)", cpu->memory_opcode_read(addr + 2),
                         cpu->memory_opcode_read(addr + 1));
        str += curlen;
        len += curlen;

        if (interpret)
        {
            value_u16 = (cpu->memory_opcode_read(addr + 2) << 8) |
                        cpu->memory_opcode_read(addr + 1);
            value_u16 = cpu->memory_read(value_u16) |
                        (cpu->memory_read((value_u16 & 0xFF00) |
                                          ((value_u16 + 1) & 0x00FF)) << 8);
            curlen = sprintf(str, " ; ($%02X%02X) -> $%04X",
                             cpu->memory_opcode_read(addr + 2),
                             cpu->memory_opcode_read(addr + 1),
                             cpu->memory_opcode_read(addr));
            str += curlen;
            len += curlen;
        }

        readuint8_t += 2;
        break;
    }

    if (strlength != NULL)
    {
        *strlength = len;
    }

    return readuint8_t;
}

#else
static char InstructionParameters[256][10] =
{
/*         00           01           02           03           04           05           06           07           08           09           0A           0B           0C           0D           0E           0F         */
/* 00 */ IP_iM "BRK", IP_iX "ORA", IP_iM "ILG", IP_iM "ILG", IP_iM "ILG", IP_zP "ORA", IP_zP "ASL", IP_iM "ILG", IP_iM "PHP", IP_iM "ORA", IP_iM "ASL", IP_iM "ILG", IP_iM "ILG", IP_aB "ORA", IP_aB "ASL", IP_iM "ILG",
/* 10 */ IP_rE "BPL", IP_iY "ORA", IP_iM "ILG", IP_iM "ILG", IP_iM "ILG", IP_zX "ORA", IP_zX "ASL", IP_iM "ILG", IP_iM "CLC", IP_aY "ORA", IP_iM "ILG", IP_iM "ILG", IP_iM "ILG", IP_aX "ORA", IP_aX "ASL", IP_iM "ILG",
/* 20 */ IP_aB "JSR", IP_iX "AND", IP_iM "ILG", IP_iM "ILG", IP_zP "BIT", IP_zP "AND", IP_zP "ROL", IP_iM "ILG", IP_iM "PLP", IP_iM "AND", IP_iM "ROL", IP_iM "ILG", IP_aB "BIT", IP_aB "AND", IP_aB "ROL", IP_iM "ILG",
/* 30 */ IP_rE "BMI", IP_iY "AND", IP_iM "ILG", IP_iM "ILG", IP_iM "ILG", IP_zX "AND", IP_zX "ROL", IP_iM "ILG", IP_iM "SEC", IP_aY "AND", IP_iM "ILG", IP_iM "ILG", IP_iM "ILG", IP_aX "AND", IP_aX "ROL", IP_iM "ILG",
/* 40 */ IP_iM "RTI", IP_iX "EOR", IP_iM "ILG", IP_iM "ILG", IP_iM "ILG", IP_zP "EOR", IP_zP "LSR", IP_iM "ILG", IP_iM "PHA", IP_iM "EOR", IP_iM "LSR", IP_iM "ILG", IP_aB "JMP", IP_aB "EOR", IP_aB "LSR", IP_iM "ILG",
/* 50 */ IP_rE "BVC", IP_iY "EOR", IP_iM "ILG", IP_iM "ILG", IP_iM "ILG", IP_zX "EOR", IP_zX "LSR", IP_iM "ILG", IP_iM "CLI", IP_aY "EOR", IP_iM "ILG", IP_iM "ILG", IP_iM "ILG", IP_aX "EOR", IP_aX "LSR", IP_iM "ILG",
/* 60 */ IP_iM "RTS", IP_iX "ADC", IP_iM "ILG", IP_iM "ILG", IP_iM "ILG", IP_zP "ADC", IP_zP "ROR", IP_iM "ILG", IP_iM "PLA", IP_iM "ADC", IP_iM "ROR", IP_iM "ILG", IP_iD "JMP", IP_aB "ADC", IP_aB "ROR", IP_iM "ILG",
/* 70 */ IP_rE "BVS", IP_iY "ADC", IP_iM "ILG", IP_iM "ILG", IP_iM "ILG", IP_zX "ADC", IP_zX "ROR", IP_iM "ILG", IP_iM "SEI", IP_aY "ADC", IP_iM "ILG", IP_iM "ILG", IP_iM "ILG", IP_aX "ADC", IP_aX "ROR", IP_iM "ILG",
/* 80 */ IP_iM "ILG", IP_iX "STA", IP_iM "ILG", IP_iM "ILG", IP_zP "STY", IP_zP "STA", IP_zP "STX", IP_iM "ILG", IP_iM "DEY", IP_iM "ILG", IP_iM "TXA", IP_iM "ILG", IP_aB "STY", IP_aB "STA", IP_aB "STX", IP_iM "ILG",
/* 90 */ IP_rE "BCC", IP_iY "STA", IP_iM "ILG", IP_iM "ILG", IP_zX "STY", IP_zX "STA", IP_zY "STX", IP_iM "ILG", IP_iM "TYA", IP_aY "STA", IP_iM "TXS", IP_iM "ILG", IP_iM "ILG", IP_aX "STA", IP_iM "ILG", IP_iM "ILG",
/* A0 */ IP_iM "LDY", IP_iX "LDA", IP_iM "LDX", IP_iM "ILG", IP_zP "LDY", IP_zP "LDA", IP_zP "LDX", IP_iM "ILG", IP_iM "TAY", IP_iM "LDA", IP_iM "TAX", IP_iM "ILG", IP_aB "LDY", IP_aB "LDA", IP_aB "LDX", IP_iM "ILG",
/* B0 */ IP_rE "BCS", IP_iY "LDA", IP_iM "ILG", IP_iM "ILG", IP_zX "LDY", IP_zX "LDA", IP_zY "LDX", IP_iM "ILG", IP_iM "CLV", IP_aY "LDA", IP_iM "TSX", IP_iM "ILG", IP_aX "LDY", IP_aX "LDA", IP_aY "LDX", IP_iM "ILG",
/* C0 */ IP_iM "CPY", IP_iX "CMP", IP_iM "ILG", IP_iM "ILG", IP_zP "CPY", IP_zP "CMP", IP_zP "DEC", IP_iM "ILG", IP_iM "INY", IP_iM "CMP", IP_iM "DEX", IP_iM "ILG", IP_aB "CPY", IP_aB "CMP", IP_aB "DEC", IP_iM "ILG",
/* D0 */ IP_rE "BNE", IP_iY "CMP", IP_iM "ILG", IP_iM "ILG", IP_iM "ILG", IP_zX "CMP", IP_zX "DEC", IP_iM "ILG", IP_iM "CLD", IP_aY "CMP", IP_iM "ILG", IP_iM "ILG", IP_iM "ILG", IP_aX "CMP", IP_aX "DEC", IP_iM "ILG",
/* E0 */ IP_iM "CPX", IP_iX "SBC", IP_iM "ILG", IP_iM "ILG", IP_zP "CPX", IP_zP "SBC", IP_zP "INC", IP_iM "ILG", IP_iM "INX", IP_iM "SBC", IP_iM "NOP", IP_iM "ILG", IP_aB "CPX", IP_aB "SBC", IP_aB "INC", IP_iM "ILG",
/* F0 */ IP_rE "BEQ", IP_iY "SBC", IP_iM "ILG", IP_iM "ILG", IP_iM "ILG", IP_zX "SBC", IP_zX "INC", IP_iM "ILG", IP_iM "SED", IP_aY "SBC", IP_iM "ILG", IP_iM "ILG", IP_iM "ILG", IP_aX "SBC", IP_aX "INC", IP_iM "ILG"
/*         00           01           02           03           04           05           06           07           08           09           0A           0B           0C           0D           0E           0F         */
};

/** Get current instruction name at specified address and put it into buffer */
int quick6502_getinstruction(quick6502_cpu *cpu, char interpret,
                             uint16_t addr, char *buffer, int *strlength)
{
   uint8_t instr = cpu->memory_opcode_read(cpu->reg_PC);
   uint8_t *instrText = InstructionParameters[instr];
   
   buffer += strlen(strcpy(buffer, instrText[1]));
   switch(instrText[0])
   {
      case IP_nPc: default: break;
      case IP_iMc: buffer += strlen(sprintf(buffer, IPf_iM, cpu->memory_opcode_read(cpu->reg_PC + 1))); break;
      case IP_iXc: buffer += strlen(sprintf(buffer, IPf_iX, cpu->memory_opcode_read(cpu->reg_PC + 1))); break;
      case IP_iYc: buffer += strlen(sprintf(buffer, IPf_iY, cpu->memory_opcode_read(cpu->reg_PC + 1))); break;
      case IP_zPc: buffer += strlen(sprintf(buffer, IPf_zP, cpu->memory_opcode_read(cpu->reg_PC + 1))); break;
      case IP_zXc: buffer += strlen(sprintf(buffer, IPf_zX, cpu->memory_opcode_read(cpu->reg_PC + 1))); break;
      case IP_zYc: buffer += strlen(sprintf(buffer, IPf_zY, cpu->memory_opcode_read(cpu->reg_PC + 1))); break;
      case IP_iDc: buffer += strlen(sprintf(buffer, IPf_iD, cpu->memory_opcode_read(cpu->reg_PC + 2), cpu->memory_opcode_read(cpu->reg_PC + 1))); break;
      case IP_aBc: buffer += strlen(sprintf(buffer, IPf_aB, cpu->memory_opcode_read(cpu->reg_PC + 2), cpu->memory_opcode_read(cpu->reg_PC + 1))); break;
      case IP_aXc: buffer += strlen(sprintf(buffer, IPf_aX, cpu->memory_opcode_read(cpu->reg_PC + 2), cpu->memory_opcode_read(cpu->reg_PC + 1))); break;
      case IP_aYc: buffer += strlen(sprintf(buffer, IPf_aY, cpu->memory_opcode_read(cpu->reg_PC + 2), cpu->memory_opcode_read(cpu->reg_PC + 1))); break;
      case IP_rEc: buffer += strlen(sprintf(buffer, IPf_rE, 0, cpu->reg_PC + (signed char) cpu->memory_opcode_read(cpu->reg_PC) + 1)); break;
   }
   
   *buffer = 0;
}

#endif

static inline int quick6502_exec_one(quick6502_cpu *cpu)
{
    register uint8_t opcode = cpu->memory_opcode_read(cpu->reg_PC);

    //char instr[100];
    //quick6502_dump(cpu, stdout);
    cpu->reg_PC++;
    TRACEi(("Quick6502: PC:$%04X A:$%02X X:$%02X Y:$%02X S:$%02X P:$%02X P:[%c%c%c%c%c%c%c%c]",
            cpu->reg_PC, cpu->reg_A, cpu->reg_X, cpu->reg_Y, cpu->reg_S, cpu->reg_P,
            cpu->reg_P & Q6502_N_FLAG ? 'N' : '.',
            cpu->reg_P & Q6502_V_FLAG ? 'V' : '.',
            cpu->reg_P & Q6502_R_FLAG ? 'R' : '.',
            cpu->reg_P & Q6502_B_FLAG ? 'B' : '.',
            cpu->reg_P & Q6502_D_FLAG ? 'D' : '.',
            cpu->reg_P & Q6502_I_FLAG ? 'I' : '.',
            cpu->reg_P & Q6502_Z_FLAG ? 'Z' : '.',
            cpu->reg_P & Q6502_C_FLAG ? 'C' : '.'));
    InstructionTable[opcode](cpu);
    //printf("--------------------------------------------------------------\n");
    /*quick6502_getinstruction(cpu, (1==1), cpu->reg_PC, instr, NULL);
    printf("%04X: %s\n", cpu->reg_PC, instr);*/

    cpu->cycle_done += CycleTable[opcode];
    if (cpu->page_crossed)
    {
        cpu->cycle_done++;
        cpu->page_crossed = 0;
    }
    if (cpu->int_pending != 0)
    {
        quick6502_int(cpu, Q6502_IRQ_SIGNAL);
    }
    return 0;
}
