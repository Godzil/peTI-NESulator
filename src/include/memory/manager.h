/*
 *  6502 Memory manager - The peTI-NESulator Project
 *  memory.h - Taken from the Quick6502 project
 *
 *  Created by Manoel Trapier on 18/09/06.
 *  Copyright 2003-2008 986 Corp. All rights reserved.
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

typedef uint8_t (*func_rdhook)(uint8_t /* addr */);
typedef void (*func_wrhook)(uint8_t addr, uint8_t data);

/* Functions to manage pages data */
void set_page_ptr(uint8_t page, uint8_t *ptr);
void set_page_ptr_1k(uint8_t page, uint8_t *ptr);
void set_page_ptr_2k(uint8_t page, uint8_t *ptr);
void set_page_ptr_4k(uint8_t page, uint8_t *ptr);
void set_page_ptr_8k(uint8_t page, uint8_t *ptr);
void set_page_ptr_16k(uint8_t page, uint8_t *ptr);
void set_page_ptr_32k(uint8_t page, uint8_t *ptr);

uint8_t *get_page_ptr(uint8_t page);


/* Functions to set pages attributes */

void set_page_rd_hook(uint8_t page, func_rdhook func);

void set_page_wr_hook(uint8_t page, func_wrhook func);

void set_page_readable(uint8_t page, uint8_t value);

void set_page_writeable(uint8_t page, uint8_t value);

void set_page_ghost(uint8_t page, uint8_t value, uint8_t ghost);

uint8_t get_page_attributes(uint8_t page);

func_rdhook get_page_rdhook(uint8_t page);

func_wrhook get_page_wrhook(uint8_t page);

/* Generalist functions */

void InitMemory();

uint8_t ReadMemory(uint8_t page, uint8_t addr);
void WriteMemory(uint8_t page, uint8_t addr, uint8_t value);

void DumpMemoryState(FILE *fp);

#endif
