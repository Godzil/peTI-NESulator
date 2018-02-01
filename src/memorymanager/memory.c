/*
 *  6502 Memory manager - The peTI-NESulator Project
 *  memory.c - Taken from the Quick6502 project
 *
 *  Created by Manoel Trapier on 18/09/06.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 */
 
#include <stdio.h>
#include <types.h>

#include <os_dependent.h>

#include <memory/manager.h>

/* Private structures */

#define Kuint8_t * (1024)

/* 
* What inside memory manager:
 *
 * Table of attributes
 * Table of original page ptr
 * Table of moded page ptr
 * 
 */

/* Private data */

uint8_t *memory_pages[0x100];
uint8_t memory_pages_attr[0x100];

func_rdhook rdh_table[0x100];
func_wrhook wrh_table[0x100];

//#define DEBUG
#undef DEBUG

#ifdef DEBUG
#define LOG(s) s
#else
#define LOG(s) {}
#endif

/* Public functions */
void set_page_ptr(uint8_t page, uint8_t *ptr)
{
    LOG(console_printf(Console_Default, "Set page 0x%X to ptr %p\n", page, ptr));
    memory_pages[page] = ptr;
}

void set_page_ptr_1k(uint8_t page, uint8_t *ptr)
{   /* 1k = 4 * 256 */
    LOG(console_printf(Console_Default, "Set page(1k) 0x%X to ptr %p\n", page, ptr));
    memory_pages[page + 0] = ptr;
    memory_pages[page + 1] = ptr +  0x100;
    memory_pages[page + 2] = ptr + (0x100 * 2);
    memory_pages[page + 3] = ptr + (0x100 * 3);
}

void set_page_ptr_2k(uint8_t page, uint8_t *ptr)
{
    LOG(console_printf(Console_Default, "Set page(2k) 0x%X to ptr %p\n", page, ptr));
    memory_pages[page + 0] = ptr;
    memory_pages[page + 1] = ptr +  0x100;
    memory_pages[page + 2] = ptr + (0x100 * 2);
    memory_pages[page + 3] = ptr + (0x100 * 3);
    memory_pages[page + 4] = ptr + (0x100 * 4);
    memory_pages[page + 5] = ptr + (0x100 * 5);
    memory_pages[page + 6] = ptr + (0x100 * 6);
    memory_pages[page + 7] = ptr + (0x100 * 7);
}

void set_page_ptr_4k(uint8_t page, uint8_t *ptr)
{
    LOG(console_printf(Console_Default, "Set page(4k) 0x%X to ptr %p\n", page, ptr));
    set_page_ptr_2k(page, ptr);
    set_page_ptr_2k(page+((4 Kuint8_t / 256) / 2), ptr + 2 Kuint8_t);
}

void set_page_ptr_8k(uint8_t page, uint8_t *ptr)
{
    LOG(console_printf(Console_Default, "Set page(8k) 0x%X to ptr %p\n", page, ptr));
    set_page_ptr_4k(page, ptr);
    set_page_ptr_4k(page+((8 Kuint8_t / 256) / 2), ptr + 4 Kuint8_t);
}

void set_page_ptr_16k(uint8_t page, uint8_t *ptr)
{
    set_page_ptr_8k(page, ptr);
    set_page_ptr_8k(page+((16 Kuint8_t / 256) / 2), ptr + 8 Kuint8_t);
}

void set_page_ptr_32k(uint8_t page, uint8_t *ptr)
{
    set_page_ptr_16k(page, ptr);
    set_page_ptr_16k(page+((32 Kuint8_t / 256) / 2), ptr + 16 Kuint8_t);
}

uint8_t *get_page_ptr(uint8_t page)
{
    return memory_pages[page];
}


/* Functions to set pages attributes */

void set_page_rd_hook(uint8_t page, func_rdhook func)
{
    if (func == NULL)
    {
        memory_pages_attr[page] &= (~ATTR_PAGE_HAVE_RDHOOK);
        if (memory_pages[page] == (uint8_t *)0x01)
            memory_pages[page] = NULL;
    }
    else
    {
        memory_pages_attr[page] |= ATTR_PAGE_HAVE_RDHOOK;
        if (memory_pages[page] == NULL)
            memory_pages[page] = (uint8_t *)0x01;
    }
    
    rdh_table[page] = func;
}

void set_page_wr_hook(uint8_t page, func_wrhook func)
{
    if (func == NULL)
    {
        memory_pages_attr[page] &= (~ATTR_PAGE_HAVE_WRHOOK);
        if (memory_pages[page] == (uint8_t*)0x01)
            memory_pages[page] = NULL;

    }
    else
    {
        memory_pages_attr[page] |= ATTR_PAGE_HAVE_WRHOOK;
        if (memory_pages[page] == NULL)
            memory_pages[page] = (uint8_t *)0x01;
    }
    
    wrh_table[page] = func;    
}

void set_page_readable(uint8_t page, uint8_t value)
{
    if (value == true)
        memory_pages_attr[page] |= ATTR_PAGE_READABLE;
    else
        memory_pages_attr[page] &= (~ATTR_PAGE_READABLE);    
}

void set_page_writeable(uint8_t page, uint8_t value)
{
    if (value == true)
        memory_pages_attr[page] |= ATTR_PAGE_WRITEABLE;
    else
        memory_pages_attr[page] &= (~ATTR_PAGE_WRITEABLE);        
}

void set_page_ghost(uint8_t page, uint8_t value, uint8_t ghost)
{
    if (value == true)
    {
        memory_pages_attr[page] = memory_pages_attr[ghost];
        rdh_table[page] = rdh_table[ghost];
        wrh_table[page] = wrh_table[ghost];
        memory_pages[page] = memory_pages[ghost];
    }
}

uint8_t get_page_attributes(uint8_t page)
{
    return memory_pages_attr[page];
}

func_rdhook get_page_rdhook(uint8_t page)
{
    if (memory_pages_attr[page] & ATTR_PAGE_HAVE_RDHOOK)
        return rdh_table[page];
    
    return NULL;
}

func_wrhook get_page_wrhook(uint8_t page)
{
    if (memory_pages_attr[page] & ATTR_PAGE_HAVE_WRHOOK)
        return wrh_table[page];
    
    return NULL;
}

uint8_t ReadMemory(uint8_t page, uint8_t addr)
{
    static uint8_t LastRetuint8_t = 0xA5;
    uint8_t *page_ptr;
    uint8_t attributes;
    LOG(console_printf(Console_Default, "Read @ 0x%X-%X\n", page, addr));
    /* Est-ce que la page est mappé ? && Est-ce que la page est "readable" ? */
    if ((page_ptr = memory_pages[page]) &&
          ( (attributes = memory_pages_attr[page]) & ATTR_PAGE_READABLE) )
    {
        LOG(console_printf(Console_Default, "Page is non null & readable\n"));
        if ( attributes & ATTR_PAGE_HAVE_RDHOOK )
            return ( LastRetuint8_t = rdh_table[page](addr) );
        else
            return ( LastRetuint8_t = page_ptr[addr] );
    }
    //console_printf(Console_Default, "Trying to read @ 0x%X-%X\n", page, addr);
    return LastRetuint8_t;
}

void WriteMemory(uint8_t page, uint8_t addr, uint8_t value)
{
    uint8_t *page_ptr;
    uint8_t attributes;
    LOG(console_printf(Console_Default, "Write 0x%x @ 0x%X-%X\n", value, page, addr));
    /* Est-ce que la page est mappé ? && Est-ce que la page est "writable" ? */
    if ( (page_ptr = memory_pages[page]) &&
         ( (attributes = memory_pages_attr[page]) & ATTR_PAGE_WRITEABLE) )
    {
        if ( attributes & ATTR_PAGE_HAVE_WRHOOK )
        {
#ifdef DETECT_BUS_CONFLICT
            if ((page >= 0x80) && (memory_pages[page][addr] != value))
                console_printf(Console_Default, "WriteHook: bus conflict at %02X%02X rom:%02X write:%02X\n", page, addr, memory_pages[page][addr], value);
#endif
            wrh_table[page](addr, value);
        }
        else
            page_ptr[addr] = value;
    }
    else { console_printf(Console_Default, "Trying to write 0x%X @ 0x%X-%X\n", value, page, addr); }
}

void DumpMemoryState(FILE *fp)
{
    int i;
    
    for (i = 0x00; i < 0x100; i++)
    {
        fprintf(fp,
                "Page 0x%02X : [%c%c%c%c%c%c] RdH:%p WrH:%p ptr:%p\n",
                i,
                (memory_pages_attr[i]&ATTR_PAGE_HAVE_RDHOOK)?'r':'.',
                (memory_pages_attr[i]&ATTR_PAGE_HAVE_WRHOOK)?'w':'.',
                (memory_pages_attr[i]&ATTR_PAGE_READABLE)?'R':'.',
                (memory_pages_attr[i]&ATTR_PAGE_WRITEABLE)?'W':'.',                        
                (memory_pages_attr[i]&ATTR_PAGE_GHOST)?'G':'.',
                (memory_pages_attr[i]&ATTR_PAGE_MAPPED)?'M':'.',
                rdh_table[i],
                wrh_table[i],
                memory_pages[i]
                );
    }
}

void InitMemory()
{
    int page;
    
    for(page = 0 ; page < 0x100 ; page++)
    {
        set_page_ptr(page,NULL);
        memory_pages_attr[page] = 0x00;
    }
}
