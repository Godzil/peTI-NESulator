/**
 *  CoreCPU - The Quick6502 Project
 *  corecpu.h
 *
 *  Created by Manoel Trapier on 24/02/08
 *  Copyright (c) 2003-2016 986-Studio. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

#ifndef _QUICK6502_CORECPU_H_
#define _QUICK6502_CORECPU_H_

/* M6502 configuration
 *
 * Supported DEFINEs :
 * Q6502_NO_DECIMAL     Quick6502 will not support BDC arithemtic (used for NES)
 * Q6502_CMOS           Quick6502 will act as a CMOS 6502 (Not actually used)
 * Q6502_DEBUGGER       Quick6502 will be build with debugguer support. Add some KB to the binary 
 *                      and may slowdown a bit the emulation.
 *
 */

#ifdef Q6502_CMOS
//#warning Quick6502 CMOS support is actually desactivated, desactivate it
#undef Q6502_CMOS
#endif

#ifndef Q6502_NO_DECIMAL
//#warning Quick6502 have actually no BCD support, fallback to no NO_DECIMAL
#define Q6502_NO_DECIMAL
#endif

#include "types.h"

typedef byte (*quick6502_MemoryReadFunction)(unsigned short addr);
typedef void (*quick6502_MemoryWriteFunction)(unsigned short addr, byte value);

typedef struct quick6502_cpu_
{
   /* 6502 registers */
   byte reg_A, reg_X, reg_Y;
   byte reg_P, reg_S;
   unsigned short reg_PC;

   /* Read/Write memory functions */
   quick6502_MemoryReadFunction  memory_read;
   quick6502_MemoryWriteFunction memory_write;
   quick6502_MemoryReadFunction  memory_page0_read;
   quick6502_MemoryWriteFunction memory_page0_write;
   quick6502_MemoryReadFunction  memory_stack_read;
   quick6502_MemoryWriteFunction memory_stack_write;
   quick6502_MemoryReadFunction  memory_opcode_read;

   /* Timing related */
   long cycle_done;
   byte exit_loop;
   byte int_pending;

   /* Other config options */
   byte running; /* This field is used to prevent cpu free if this cpu is running */
   byte page_crossed;
   
   /* TODO add support for Inst/MemAccess breakpoints */
   
} quick6502_cpu;

typedef struct quick6502_cpuconfig_
{
   /* Read/Write memory functions */
   quick6502_MemoryReadFunction  memory_read;
   quick6502_MemoryWriteFunction memory_write;
   quick6502_MemoryReadFunction  memory_page0_read;
   quick6502_MemoryWriteFunction memory_page0_write;
   quick6502_MemoryReadFunction  memory_stack_read;
   quick6502_MemoryWriteFunction memory_stack_write;
   quick6502_MemoryReadFunction  memory_opcode_read;
} quick6502_cpuconfig;

/*** Signal that we can send to the CPU ***/
typedef enum
{
   Q6502_NO_SIGNAL = 0,
   Q6502_IRQ_SIGNAL,
   Q6502_NMI_SIGNAL,
   Q6502_STOPLOOP_SIGNAL
} quick6502_signal;

/*** Some 6502 related definitions ***/

/*** P register flags ***/
#define Q6502_N_FLAG 0x80  /* Negavite flag */
#define Q6502_V_FLAG 0x40  /* oVerflow flag */
#define Q6502_R_FLAG 0x20  /* Not a real flag, but need to be to 1 on PHP */
#define Q6502_B_FLAG 0x10  /* Break flag    */
#define Q6502_D_FLAG 0x08  /* BCD flag      */
#define Q6502_I_FLAG 0x04  /* IRQ/BRK flag  */
#define Q6502_Z_FLAG 0x02  /* Zero flag     */
#define Q6502_C_FLAG 0x01  /* Carry flag    */

/*** Interuption Vectors ***/
#define Q6502_NMI_LOW    0xFFFA
#define Q6502_NMI_HIGH   0xFFFB
#define Q6502_RESET_LOW  0xFFFC
#define Q6502_RESET_HIGH 0xFFFD
#define Q6502_IRQ_LOW    0xFFFE
#define Q6502_IRQ_HIGH   0xFFFF

/**
 * Initialise the CPU 
 *
 * Inputs:
 *
 * - CPU Init structure:
 * +- Memory Read function pointer
 * +- Memory Write function pointer
 * +- Fast memory read function pointer (for opcodes read)
 * +- Fast page 0 function / Read/Write
 * +- Fast page 1 function / Read/Write
 *
 * Output:
 *
 * (void *): An opaque pointer to the internal structure of the CPU
 *
 */
quick6502_cpu *quick6502_init(quick6502_cpuconfig *config);

/* Reset the CPU (must be done after init) */
void quick6502_reset(quick6502_cpu *cpu);

/**
 * Run cpu for at least X cycles 
 *
 * Output:
 *
 * int: (Number of cycle really done) - (Number of cycle asked)
 */
int quick6502_run(quick6502_cpu *cpu, int cycles);

/** Loop CPU until explicit quit */
void quick6502_loop(quick6502_cpu *cpu);

/** Run CPU for one instruction */
void quick6502_exec(quick6502_cpu *cpu);

/** Send IRQ/NMI/EXITLOOP signal to CPU */
void quick6502_int(quick6502_cpu *cpu, quick6502_signal signal);

/** Dump CPU State to the given file */
void quick6502_dump(quick6502_cpu *cpu, FILE * fp);

/** Get current instruction name at specified address and put it into buffer */
#define MINE

int quick6502_getinstruction(quick6502_cpu *cpu, char interpret,
                             unsigned short addr, char *buffer, int *strlength);                        
/**
 * Free the CPU 
 *
 * This function will free the CPU only if it's not currently used, it will
 * return !0 if everything goes well and 0 if the free is impossible
 */
int quick6502_free(quick6502_cpu *cpu);

#endif /* _QUICK6502_CORECPU_H_ */

