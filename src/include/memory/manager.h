/*
 *  6502 Memory manager - The TI-NESulator Project
 *  memory.h - Taken from the Quick6502 project
 *
 *  Created by Manoel Trapier on 18/09/06.
 *  Copyright 2003-2008 986 Corp. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */
#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

#define ATTR_PAGE_HAVE_RDHOOK    0x20
#define ATTR_PAGE_HAVE_WRHOOK    0x10
#define ATTR_PAGE_WRITEABLE      0x08
#define ATTR_PAGE_READABLE       0x04
#define ATTR_PAGE_GHOST          0x02
#define ATTR_PAGE_MAPPED         0x01

typedef byte (*func_rdhook)(byte /* addr */);
typedef void (*func_wrhook)(byte addr, byte data);

/* Functions to manage pages data */
void set_page_ptr(byte page, byte *ptr);
void set_page_ptr_1k(byte page, byte *ptr);
void set_page_ptr_2k(byte page, byte *ptr);
void set_page_ptr_4k(byte page, byte *ptr);
void set_page_ptr_8k(byte page, byte *ptr);
void set_page_ptr_16k(byte page, byte *ptr);
void set_page_ptr_32k(byte page, byte *ptr);

byte *get_page_ptr(byte page);


/* Functions to set pages attributes */

void set_page_rd_hook(byte page, func_rdhook func);    

void set_page_wr_hook(byte page, func_wrhook func);

void set_page_readable(byte page, bool value);

void set_page_writeable(byte page, bool value);

void set_page_ghost(byte page, bool value, byte ghost);

byte get_page_attributes(byte page);

func_rdhook get_page_rdhook(byte page);

func_wrhook get_page_wrhook(byte page);

/* Generalist functions */

void InitMemory();

byte ReadMemory(byte page, byte addr);
void WriteMemory(byte page, byte addr, byte value);

void DumpMemoryState(FILE *fp);

#endif
