/*
 *  Code Breaker plugin - The peTI-NESulator Project
 *  gamegenie.c: Hack your games with unlimited lives of add new powers!
 *
 *  Created by Manoël Trapier.
 *  Copyright (c) 2002-2019 986-Studio.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <os_dependent.h>

#define __TINES_PLUGINS__

#include <plugins/manager.h>

#undef  __TINES_PLUGINS_
#include <os_dependent.h>

#include <memory/manager.h>
#include <types.h>

typedef enum gg_States_
{
    GG_S00_MAIN_STATE = 0,
    GG_S01_SEARCH_VALUE,
    GG_S02_SEARCH_BAR
} gg_States;

/* Actual State Machine state */
gg_States gg_state = GG_S00_MAIN_STATE;

/* Own representation of memory */
uint8_t gg_MainRAM[0x800];
uint8_t gg_OldMainRAM[0x800];
uint8_t gg_SRAM[0x2000];

/* Field used to now which uint8_t are currently marked as pertinent or not */
uint8_t gg_use_MainRAM[0x800];
uint8_t gg_use_SRAM[0x2000];

int gg_ResultNumber;

uint8_t gg_PatchUsed[10];
uint8_t gg_PatchedPage[10];
uint8_t gg_PatchedAddr[10];
uint8_t gg_PatchedValue[10];
func_rdhook gg_rdhookPtr[10];

#define GG_RDHOOKPATCH(d) \
static uint8_t gg_RdHookPatch##d(uint8_t addr) \
{ \
    if (addr == gg_PatchedAddr[d]) \
    { \
        return gg_PatchedValue[d]; \
    } \
    else \
    { \
        if (gg_rdhookPtr[d] != NULL) \
            return gg_rdhookPtr[d](addr); \
        else \
            return (get_page_ptr(gg_PatchedPage[d])[addr]); \
    } \
}

#define GG_MAX_PATCH 10
/* Defines the read hook patches */
GG_RDHOOKPATCH(0)
GG_RDHOOKPATCH(1)
GG_RDHOOKPATCH(2)
GG_RDHOOKPATCH(3)
GG_RDHOOKPATCH(4)
GG_RDHOOKPATCH(5)
GG_RDHOOKPATCH(6)
GG_RDHOOKPATCH(7)
GG_RDHOOKPATCH(8)
GG_RDHOOKPATCH(9)

void gg_SetPatch(int id, uint8_t page, uint8_t addr, uint8_t value)
{
    func_rdhook fptr;
    func_rdhook cur_ptr;

    if (id >= GG_MAX_PATCH)
    {
        return;
    }

    /* Set parameters for the patch */
    if (gg_PatchUsed[id] == 0x00)
    {
        gg_rdhookPtr[id] = get_page_rdhook(page);
    }

    gg_PatchedPage[id] = page;
    gg_PatchedAddr[id] = addr;
    gg_PatchedValue[id] = value;
    gg_PatchUsed[id] = 0xFF;

    /* Set a ReadHook on the page */

    switch (id)
    {
    default:
    case 0:
        fptr = gg_RdHookPatch0;
        break;

    case 1:
        fptr = gg_RdHookPatch1;
        break;

    case 2:
        fptr = gg_RdHookPatch2;
        break;

    case 3:
        fptr = gg_RdHookPatch3;
        break;

    case 4:
        fptr = gg_RdHookPatch4;
        break;

    case 5:
        fptr = gg_RdHookPatch5;
        break;

    case 6:
        fptr = gg_RdHookPatch6;
        break;

    case 7:
        fptr = gg_RdHookPatch7;
        break;

    case 8:
        fptr = gg_RdHookPatch8;
        break;

    case 9:
        fptr = gg_RdHookPatch9;
        break;
    }

    cur_ptr = get_page_rdhook(page);
    if (cur_ptr != fptr)
    {
        set_page_rd_hook(page, fptr);
    }
}

void MessageBox(char *title, char *msg)
{
    int sc_w, sc_h;
    int box_h, box_t, box_l, box_w;

    sc_w = 640; //screen->w;
    sc_h = 480; //screen->h;

    /*gg_Buffer = create_bitmap(sc_w, sc_h);

    blit(Buffer, gg_Buffer, 0, 0, 0, 0, 512 + 256, 480);*/

    box_w = 0;// text_length(font, title) + 10;

    //box_w = (box_w > text_length(font, msg)) ? box_w : text_length(font, msg);

    box_w += 15 * 2; /*sc_w/2;*/
    box_h = 15 * 2 + 10;

    /* Set the box center */
    box_t = (sc_h - box_h) / 2;
    box_l = (sc_w - box_w) / 2;

    graphics_drawFillrect(box_l, box_t, box_l + box_w, box_t + box_h, 60);
    graphics_drawRect(box_l + 5, box_t + 5, box_l + box_w - 5, box_t + box_h - 5, 34);

    /* Display the title */
    //textout_centre_ex(gg_Buffer, font, title, box_w / 2 + box_l, box_t + 2, 34, 60);

    /* Display the message */
    //textout_centre_ex(gg_Buffer, font, msg, box_w / 2 + box_l, 15 + box_t + 2, 34, 60);

    //blit(gg_Buffer, screen, 0, 0, 0, 0, 512 + 256, 480);

    sleep(1000);

    //release_bitmap(gg_Buffer);

}

uint16_t SelectNumber(char *title, char *msg, uint8_t size)
{

    //int sc_w, sc_h;
    //int box_h;
    int box_w;
    //int box_t, box_l;

    char valueText[10];

    uint16_t value;
    uint8_t digit = 0;

    //sc_w = 640; //screen->w;
    //sc_h = 480; //screen->h;

    //gg_Buffer = create_bitmap(sc_w, sc_h);

    //blit(Buffer, gg_Buffer, 0, 0, 0, 0, 512 + 256, 480);

    //box_w = text_length(font, title) + 10;

    box_w = 0; //(box_w > text_length(font, msg)) ? box_w : text_length(font, msg);

    sprintf(valueText, "0000");

    //box_w = (box_w > text_length(font, valueText)) ? box_w : text_length(font, msg);

    box_w += 15 * 2; /*sc_w/2;*/
    //box_h = 15 * 2 + 30;

    /* Set the box center */
    //box_t = (sc_h - box_h) / 2;
    //box_l = (sc_w - box_w) / 2;


    value = 0;

    while (getKeyStatus(KEY_ENTER)) // ENTER
    {

        //rectfill(gg_Buffer, box_l, box_t, box_l + box_w, box_t + box_h, 60);
        //rect(gg_Buffer, box_l + 5, box_t + 5, box_l + box_w - 5, box_t + box_h - 5, 34);

        /* Display the title */
        //textout_centre_ex(gg_Buffer, font, title, box_w / 2 + box_l, box_t + 2, 34, 60);

        /* Display the message */
        //textout_centre_ex(gg_Buffer, font, msg, box_w / 2 + box_l, 15 + box_t + 2, 34, 60);

        if (size == 2)
        {
            sprintf(valueText, "  %02X", value & 0xFF);
        }
        else
        {
            sprintf(valueText, "%04X", value);
        }

        //textout_centre_ex(gg_Buffer, font, valueText, box_w / 2 + box_l, 15 + box_t + 2 + 10, 34, 60);

        switch (digit)
        {
        default:
        case 0:
            //textout_centre_ex(gg_Buffer, font, "   ^", box_w / 2 + box_l, 15 + box_t + 2 + 20, 34, 60);
            break;
        case 1:
            //textout_centre_ex(gg_Buffer, font, "  ^ ", box_w / 2 + box_l, 15 + box_t + 2 + 20, 34, 60);
            break;

        case 2:
            //textout_centre_ex(gg_Buffer, font, " ^  ", box_w / 2 + box_l, 15 + box_t + 2 + 20, 34, 60);
            break;

        case 3:
            //textout_centre_ex(gg_Buffer, font, "^   ", box_w / 2 + box_l, 15 + box_t + 2 + 20, 34, 60);
            break;
        }

        //blit(gg_Buffer, screen, 0, 0, 0, 0, 512 + 256, 480);

        if (getKeyStatus(KEY_UP)) // UP
        {
            usleep(100000);
            value += ((digit == 0) ? 0x0001 : ((digit == 1) ? 0x0010 : ((digit == 2) ? 0x0100 : 0x1000)));
            value &= (size == 2) ? 0xFF : 0xFFFF;
        }

        if (getKeyStatus(KEY_DOWN)) // DOWN
        {
            usleep(100000);
            value -= ((digit == 0) ? 0x0001 : ((digit == 1) ? 0x0010 : ((digit == 2) ? 0x0100 : 0x1000)));
            value &= (size == 2) ? 0xFF : 0xFFFF;
        }

        if (getKeyStatus(KEY_RIGHT)) // RIGHT
        {
            usleep(100000);
            if (digit <= 0)
            {
                digit = size - 1;
            }
            else
            {
                digit--;
            }
        }

        if (getKeyStatus(KEY_LEFT))
        {
            usleep(100000);
            if (digit >= size - 1)
            {
                digit = 0;
            }
            else
            {
                digit++;
            }
        }

    }
    //release_bitmap(gg_Buffer);
    while (getKeyStatus(KEY_ENTER))
    {
    }
    return value;
}

int DispMenu(int itemc, char *itemv[], char *title)
{
    //console_printf(Console_Default, "%s(%d, %p, \"%s\");\n", __func__, itemc, itemv, title);

    int selection = 0;
    int i;
    int sc_w, sc_h;
    int32_t box_h, box_t, box_l, box_w;
    int32_t text_h;

    graphics_getScreenSize(&sc_w, &sc_h);

    //gg_Buffer = create_bitmap(sc_w, sc_h);

    //blit(Buffer, gg_Buffer, 0, 0, 0, 0, 512 + 256, 480);

    graphics_get_text_size(&box_w, &text_h, NULL, title);
    box_w += 10;


    for (i = 0 ; i < itemc ; i++)
    {
        int32_t tmp;
        graphics_get_text_size(&tmp, NULL, NULL, itemv[i]);
        if (box_w < tmp)
        {
            box_w = tmp;
        }
    }

    box_w += 15 * 2; /*sc_w/2;*/
    box_h = 15 * 2 + itemc * 10;

    /* Set the box center */
    box_t = (sc_h - box_h) / 2;
    box_l = (sc_w - box_w) / 2;


    while (!getKeyStatus(KEY_ENTER))
    {
        /* Draw the box and highlight the selected item */
        int i;
        for (i = 0; i < box_h; i++)
        {
            graphics_drawline(box_l, box_t+i, box_w, box_t + i, 1);
        }
        graphics_drawline(5, 121, 251, 121, 41);

        //graphics_drawFillrect(box_l, box_t, box_w, box_h, 5);
        //graphics_drawRect(box_l + 5, box_t + 5, box_w - 5, box_h - 5, 1);

        /* Display the title */
        graphics_text_ex(box_l, box_t + 2, box_w, text_h,
                         NULL,
                         34, 60,
                         TEXT_VALIGN_CENTER, TEXT_HALIGN_CENTER,
                         0,
                         title);
        //textout_centre_ex(gg_Buffer, font, title, box_w / 2 + box_l, box_t + 2, 34, 60);

        /* Display the highlight item */
        //graphics_drawFillrect(box_l + 15, 15 + box_t + (selection * 10), box_l + box_w - 15,
        //        15 + box_t + (selection * 10) + 10, 34);
        graphics_draw_text(box_w / 2 + box_l, 15 + box_t + (selection * 10) + 2, 60, NULL, itemv[selection]);
        //textout_centre_ex(gg_Buffer, font, itemv[selection], box_w / 2 + box_l, 15 + box_t + (selection * 10) + 2, 60,
        //                  34);

        /* Display other items */
        for (i = 0 ; i < itemc ; i++)
        {
            if (i != selection)
            {
                //textout_centre_ex(gg_Buffer, font, itemv[i], box_w / 2 + box_l, 15 + box_t + (i * 10) + 2, 34, 60);
            }
        }


        /* Blit the screen buffer */
        vsync();
        //blit(gg_Buffer, screen, 0, 0, 0, 0, 512 + 256, 480);

        /* Now get the keyboard state */
        if (getKeyStatus(KEY_UP))
        {
            usleep(100000);
            if (selection <= 0)
            {
                selection = itemc - 1;
            }
            else
            {
                selection--;
            }
        }

        if (getKeyStatus(KEY_DOWN))
        {
            usleep(100000);
            if (selection >= (itemc - 1))
            {
                selection = 0;
            }
            else
            {
                selection++;
            }
        }

    }

    //release_bitmap(gg_Buffer);
    while (getKeyStatus(KEY_ENTER))
    {
        vsync();
    }
    return selection;
}

uint8_t AskYesNo(char *title)
{
    char *YesNo[] = { "No", "Yes" };

    return DispMenu(2, YesNo, title);
}

uint8_t gg_CalcChk(uint16_t addr, uint8_t value)
{
    int chk = 0x42;
    chk += (addr & 0xFF00) >> 8;
    chk -= (addr & 0x00FF);
    chk += (value & 0x00FF);
    return chk;
}

/* 
   Code is AAAAVVCC where 
   AAAA = address,
   VV   = value,
   CC   = cheksum
 */
uint32_t gg_MakeCode(uint16_t addr, uint8_t value)
{
    uint32_t code = addr << 16;
    code |= (value << 8);
    code |= (gg_CalcChk(addr, value) & 0x00FF);

    return code ^ 0x246FF53A;
}

uint8_t gg_SelectPatch()
{
    char *Items[GG_MAX_PATCH + 1];
    char *tmp;
    int i;
    uint8_t ret;

    for (i = 0 ; i < GG_MAX_PATCH ; i++)
    {
        tmp = (char *)malloc(0x100);
        console_printf(Console_Default, "Items[%d]: %p\n", i, tmp);
        if (gg_PatchUsed[i] == 0x00)
        {
            sprintf(tmp, "Patch %d: Not used", i);
        }
        else
        {
            sprintf(tmp, "Patch %d: Put 0x%02X on address 0x%02X%02X (Code: %08X)",
                    i, gg_PatchedValue[i], gg_PatchedPage[i], gg_PatchedAddr[i],
                    gg_MakeCode((gg_PatchedPage[i] << 8) | gg_PatchedAddr[i], gg_PatchedValue[i]));
        }

        Items[i] = tmp;
    }

    tmp = (char *)malloc(0x100);
    sprintf(tmp, "Return");
    Items[GG_MAX_PATCH] = tmp;

    ret = DispMenu(GG_MAX_PATCH + 1, Items, "Code Breaker - Select a patch");

    for (i = 0 ; i < GG_MAX_PATCH ; i++)
    {
        free(Items[i]);
    }

    if (ret == GG_MAX_PATCH)
    {
        return 0xFF;
    }

    return ret;
}

void gg_PatchManager()
{
    console_printf(Console_Default, "DTC!\n");
}

void gg_InitSearch()
{
    uint16_t addr;

    for (addr = 0x000 ; addr < 0x800 ; addr++)
    {
        gg_MainRAM[addr] = ReadMemory((addr & 0xFF00) >> 8, addr & 0x00FF);
        gg_use_MainRAM[addr] = 0xFF;
    }

    gg_ResultNumber = 0x800;
}

typedef enum gg_SearchForMode_
{
    GG_SEARCHFOR_LOWER = 0,
    GG_SEARCHFOR_HIGHER,
    GG_SEARCHFOR_IDENTIC,
    GG_SEARCHFOR_DIFFERENT

} gg_SearchForMode;

void gg_SearchForValue(uint8_t value)
{
    uint16_t addr;
    //uint8_t oldValue;
    uint8_t currentValue;
    gg_ResultNumber = 0x00;
    for (addr = 0x000 ; addr < 0x800 ; addr++)
    {
        if (gg_use_MainRAM[addr] == 0xFF)
        {
            /* "Backup" the old ram */
            memcpy(gg_OldMainRAM, gg_MainRAM, 0x800);

            //oldValue = gg_MainRAM[addr];
            currentValue = ReadMemory((addr & 0xFF00) >> 8, addr & 0x00FF);

            if (currentValue != value)
            { /* This is not the good one ! */
                gg_use_MainRAM[addr] = 0x00;
            }
            else
            { /* This can be the good one ! */
                gg_ResultNumber++;
                gg_MainRAM[addr] = currentValue;
            }
        }
    }
}

void gg_SearchFor(gg_SearchForMode mode)
{
    uint16_t addr;
    uint8_t oldValue;
    uint8_t currentValue;
    gg_ResultNumber = 0x00;
    for (addr = 0x000 ; addr < 0x800 ; addr++)
    {
        if (gg_use_MainRAM[addr] == 0xFF)
        {
            /* "Backup" the old ram */
            memcpy(gg_OldMainRAM, gg_MainRAM, 0x800);

            oldValue = gg_MainRAM[addr];
            currentValue = ReadMemory((addr & 0xFF00) >> 8, addr & 0x00FF);

            switch (mode)
            {
            case GG_SEARCHFOR_LOWER:
                if (currentValue >= oldValue)
                { /* This is not the good one ! */
                    gg_use_MainRAM[addr] = 0x00;
                }
                else
                { /* This can be the good one ! */
                    gg_ResultNumber++;
                    gg_MainRAM[addr] = currentValue;
                }
                break;

            case GG_SEARCHFOR_HIGHER:
                if (currentValue <= oldValue)
                { /* This is not the good one ! */
                    gg_use_MainRAM[addr] = 0x00;
                }
                else
                { /* This can be the good one ! */
                    gg_ResultNumber++;
                    gg_MainRAM[addr] = currentValue;
                }
                break;

            case GG_SEARCHFOR_IDENTIC:
                if (currentValue != oldValue)
                { /* This is not the good one ! */
                    gg_use_MainRAM[addr] = 0x00;
                }
                else
                { /* This can be the good one ! */
                    gg_ResultNumber++;
                    gg_MainRAM[addr] = currentValue;
                }
                break;
            case GG_SEARCHFOR_DIFFERENT:
                if (currentValue == oldValue)
                { /* This is not the good one ! */
                    gg_use_MainRAM[addr] = 0x00;
                }
                else
                { /* This can be the good one ! */
                    gg_ResultNumber++;
                    gg_MainRAM[addr] = currentValue;
                }
                break;
            }
        }
    }
}

uint8_t gg_DisplayResults()
{
    char *Items[100];
    char *tmp;
    int i, addr = 0x0000;
    uint8_t ret = 0;

    uint16_t AddrList[21];
    if (gg_ResultNumber > 20)
    {
        MessageBox("Code Breaker", "Too many results for displaying them!");
    }
    else
    {
        for (i = 0 ; i < gg_ResultNumber ; i++)
        {
            while (gg_use_MainRAM[addr] != 0xFF)
            {
                addr++;
            }
            console_printf(Console_Default, "0x%04X [%d]\n", addr, i);
            tmp = (char *)malloc(0x100);
            sprintf(tmp, "Patch: %08XAddress 0x%04X - Was: 0x%02X - Actual: 0x%02X",
                    i,
                    addr,
                    gg_OldMainRAM[addr],
                    gg_MainRAM[addr]);
            Items[i] = tmp;
            AddrList[i] = addr;

            addr++;
        }
        tmp = (char *)malloc(0x100);
        sprintf(tmp, "Return");
        Items[i] = tmp;

        ret = DispMenu(gg_ResultNumber + 1, Items, "Code Breaker - Search");
        if (ret < i)
        {
            if (AskYesNo("Code Breaker: Apply this patch?"))
            {
                /* Now patch it ! */
                gg_SetPatch(gg_SelectPatch(), (AddrList[ret] & 0xFF00) >> 8, (AddrList[ret] & 0x00FF),
                            SelectNumber("Code Breaker", "Value to apply:", 2) & 0x00FF);
                ret = 1;
            }
        }

        for (i = 0 ; i < gg_ResultNumber + 1 ; i++)
        {
            free(Items[i]);
        }
    }

    return ret;
}

void gg_Start()
{
    char *S00_MenuList[] = { "Search a specific Value", "Search for an Unknown Value", "Enter code",
                             "Manage Patches", "Exit" };

    char *S01_MenuList[] = { "Value is identical", "New Value...", "Value is different",
                             "Value is greater", "Value is lower", "Result", "Restart", "Exit" };

    char *S02_MenuList[] = { "Value is identical", "Value is different", "Value is greater",
                             "Value is lower", "Result", "Restart", "Exit" };

    char Buffer[100];
    int ret;
    uint8_t value;
    uint16_t addr;

    console_printf(Console_Default, "Open GG plugin...\n");

    switch (gg_state)
    {
    default:
    case GG_S00_MAIN_STATE:
        ret = DispMenu(5, S00_MenuList, "Code Breaker - Main");

        switch (ret)
        {
        case 0:

            gg_InitSearch();
            gg_state = GG_S01_SEARCH_VALUE;
            value = SelectNumber("Code Breaker", "Select the value:", 2) & 0x00FF;

            gg_SearchForValue(value);

            MessageBox("Code Breaker", "Search initialized !");
            break;
        case 1:
            gg_InitSearch();
            gg_state = GG_S02_SEARCH_BAR;
            MessageBox("Code Breaker", "Search initialized !");
            break;
        case 2: /* Enter code */
            addr = SelectNumber("Code Breaker", "Select the address", 4);
            value = SelectNumber("Code Breaker", "Select the value:", 2) & 0x00FF;

            if (AskYesNo("Code Breaker: Apply this patch?"))
            {
                /* Now patch it ! */
                gg_SetPatch(gg_SelectPatch(),
                            (addr & 0xFF00) >> 8, (addr & 0x00FF),
                            value);
            }

            break;

        case 3: /* Patch manager */
            gg_PatchManager();
            break;
        }


        break;

    case GG_S01_SEARCH_VALUE:
    S01_MENU:
        ret = DispMenu(8, S01_MenuList, "Code Breaker - Search");
        switch (ret)
        {
        case 0:
            gg_SearchFor(GG_SEARCHFOR_IDENTIC);
            //goto S02_MENU;
            break;
        case 1:
            value = SelectNumber("Code Breaker", "Select the value:", 2) & 0x00FF;

            gg_SearchForValue(value);
            break;

        case 2:
            gg_SearchFor(GG_SEARCHFOR_DIFFERENT);
            //goto S02_MENU;
            break;

        case 3:
            gg_SearchFor(GG_SEARCHFOR_HIGHER);
            //goto S02_MENU;
            break;

        case 4:
            gg_SearchFor(GG_SEARCHFOR_LOWER);
            //goto S02_MENU;
            break;

        case 5: /* Results */
            if (gg_DisplayResults() == 1)
            {
                gg_state = GG_S00_MAIN_STATE;
            }
            else
            {
                goto S01_MENU;
            }
            break;

        case 6:
            if (AskYesNo("Code Breaker: Restart?"))
            {
                gg_state = GG_S00_MAIN_STATE;
                gg_Start();
            }
            else
            {
                goto S01_MENU;
            }
            break;

        }
        sprintf(Buffer, "Results found: %d", gg_ResultNumber);
        MessageBox("Code Breaker", Buffer);

        break;

    case GG_S02_SEARCH_BAR:
    S02_MENU:
        ret = DispMenu(7, S02_MenuList, "Code Breaker - Search");
        switch (ret)
        {
        case 0:
            gg_SearchFor(GG_SEARCHFOR_IDENTIC);
            //goto S02_MENU;
            break;

        case 1:
            gg_SearchFor(GG_SEARCHFOR_DIFFERENT);
            //goto S02_MENU;
            break;

        case 2:
            gg_SearchFor(GG_SEARCHFOR_HIGHER);
            //goto S02_MENU;
            break;

        case 3:
            gg_SearchFor(GG_SEARCHFOR_LOWER);
            //goto S02_MENU;
            break;

        case 4: /* Results */
            if (gg_DisplayResults() == 1)
            {
                gg_state = GG_S00_MAIN_STATE;
            }
            else
            {
                goto S02_MENU;
            }
            break;

        case 5:
            if (AskYesNo("Code Breaker: Restart?"))
            {
                gg_state = GG_S00_MAIN_STATE;
                gg_Start();
            }
            else
            {
                goto S02_MENU;
            }
            break;

        }
        sprintf(Buffer, "Results found: %d", gg_ResultNumber);
        MessageBox("Code Breaker", Buffer);
        break;
    }
}


int gg_Init()
{
    int i;
    console_printf(Console_Default, "Initializing GG plugin...\n");

    plugin_install_keypressHandler('G', gg_Start);

    for (i = 0 ; i < GG_MAX_PATCH ; i++)
    {
        gg_PatchUsed[i] = 0x00;
    }

    return 0;
}


int gg_Deinit()
{
    return 0;
}