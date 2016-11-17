/*
 *  6502 Memory manager - The TI-NESulator Project
 *  memory.c - Taken from the Quick6502 project
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
 
#include <stdio.h>
#include <types.h>

#include <os_dependent.h>

#include <memory/manager.h>

/* Private structures */

#define KBYTE * (1024)

/* 
* What inside memory manager:
 *
 * Table of attributes
 * Table of original page ptr
 * Table of moded page ptr
 * 
 */

/* Private data */

byte *memory_pages[0x100];
byte memory_pages_attr[0x100];

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
void set_page_ptr(byte page, byte *ptr)
{
    LOG(console_printf(Console_Default, "Set page 0x%X to ptr %p\n", page, ptr));
    memory_pages[page] = ptr;
}

void set_page_ptr_1k(byte page, byte *ptr)
{   /* 1k = 4 * 256 */
    LOG(console_printf(Console_Default, "Set page(1k) 0x%X to ptr %p\n", page, ptr));
    memory_pages[page + 0] = ptr;
    memory_pages[page + 1] = ptr +  0x100;
    memory_pages[page + 2] = ptr + (0x100 * 2);
    memory_pages[page + 3] = ptr + (0x100 * 3);
}

void set_page_ptr_2k(byte page, byte *ptr)
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

void set_page_ptr_4k(byte page, byte *ptr)
{
    LOG(console_printf(Console_Default, "Set page(4k) 0x%X to ptr %p\n", page, ptr));
    set_page_ptr_2k(page, ptr);
    set_page_ptr_2k(page+((4 KBYTE / 256) / 2), ptr + 2 KBYTE);    
}

void set_page_ptr_8k(byte page, byte *ptr)
{
    LOG(console_printf(Console_Default, "Set page(8k) 0x%X to ptr %p\n", page, ptr));
    set_page_ptr_4k(page, ptr);
    set_page_ptr_4k(page+((8 KBYTE / 256) / 2), ptr + 4 KBYTE);        
}

void set_page_ptr_16k(byte page, byte *ptr)
{
    set_page_ptr_8k(page, ptr);
    set_page_ptr_8k(page+((16 KBYTE / 256) / 2), ptr + 8 KBYTE);    
}

void set_page_ptr_32k(byte page, byte *ptr)
{
    set_page_ptr_16k(page, ptr);
    set_page_ptr_16k(page+((32 KBYTE / 256) / 2), ptr + 16 KBYTE);    
}

byte *get_page_ptr(byte page)
{
    return memory_pages[page];
}


/* Functions to set pages attributes */

void set_page_rd_hook(byte page, func_rdhook func)
{
    if (func == NULL)
    {
        memory_pages_attr[page] &= (~ATTR_PAGE_HAVE_RDHOOK);
        if (memory_pages[page] == (byte *)0x01)
            memory_pages[page] = NULL;
    }
    else
    {
        memory_pages_attr[page] |= ATTR_PAGE_HAVE_RDHOOK;
        if (memory_pages[page] == NULL)
            memory_pages[page] = (byte *)0x01;
    }
    
    rdh_table[page] = func;
}

void set_page_wr_hook(byte page, func_wrhook func)
{
    if (func == NULL)
    {
        memory_pages_attr[page] &= (~ATTR_PAGE_HAVE_WRHOOK);
        if (memory_pages[page] == (byte*)0x01)
            memory_pages[page] = NULL;

    }
    else
    {
        memory_pages_attr[page] |= ATTR_PAGE_HAVE_WRHOOK;
        if (memory_pages[page] == NULL)
            memory_pages[page] = (byte *)0x01;
    }
    
    wrh_table[page] = func;    
}

void set_page_readable(byte page, bool value)
{
    if (value == true)
        memory_pages_attr[page] |= ATTR_PAGE_READABLE;
    else
        memory_pages_attr[page] &= (~ATTR_PAGE_READABLE);    
}

void set_page_writeable(byte page, bool value)
{
    if (value == true)
        memory_pages_attr[page] |= ATTR_PAGE_WRITEABLE;
    else
        memory_pages_attr[page] &= (~ATTR_PAGE_WRITEABLE);        
}

void set_page_ghost(byte page, bool value, byte ghost)
{
    if (value == true)
    {
        memory_pages_attr[page] = memory_pages_attr[ghost];
        rdh_table[page] = rdh_table[ghost];
        wrh_table[page] = wrh_table[ghost];
        memory_pages[page] = memory_pages[ghost];
    }
}

byte get_page_attributes(byte page)
{
    return memory_pages_attr[page];
}

func_rdhook get_page_rdhook(byte page)
{
    if (memory_pages_attr[page] & ATTR_PAGE_HAVE_RDHOOK)
        return rdh_table[page];
    
    return NULL;
}

func_wrhook get_page_wrhook(byte page)
{
    if (memory_pages_attr[page] & ATTR_PAGE_HAVE_WRHOOK)
        return wrh_table[page];
    
    return NULL;
}

byte ReadMemory(byte page, byte addr)
{
    static byte LastRetByte = 0xA5;
    byte *page_ptr;
    byte attributes;
    LOG(console_printf(Console_Default, "Read @ 0x%X-%X\n", page, addr));
    /* Est-ce que la page est mappé ? && Est-ce que la page est "readable" ? */
    if ((page_ptr = memory_pages[page]) &&
          ( (attributes = memory_pages_attr[page]) & ATTR_PAGE_READABLE) )
    {
        LOG(console_printf(Console_Default, "Page is non null & readable\n"));
        if ( attributes & ATTR_PAGE_HAVE_RDHOOK )
            return ( LastRetByte = rdh_table[page](addr) );
        else
            return ( LastRetByte = page_ptr[addr] );
    }
    //console_printf(Console_Default, "Trying to read @ 0x%X-%X\n", page, addr);
    return LastRetByte;
}

void WriteMemory(byte page, byte addr, byte value)
{
    byte *page_ptr;
    byte attributes;
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
