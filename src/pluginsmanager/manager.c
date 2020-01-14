/*
 *  Plugins manager - The peTI-NESulator Project
 *  plugins.c
 *
 *  Created by Manoël Trapier on 02/04/07.
 *  Copyright (c) 2002-2019 986-Studio.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <os_dependent.h>

#include <plugins/manager.h>

typedef struct Plugin_
{
    char *name;

    PluginInit init;
    PluginDeinit deinit;

} Plugin;

typedef struct KeyHandler_
{
    uint8_t key;

    PluginKeypress func;

    struct KeyHandler_ *next;

} KeyHandler;

KeyHandler *keyHandlersList = NULL;

#include "plugins_list.h"

void plugin_list()
{
    Plugin *ptr = &(Plugins[0]);
    int i = 1;
    console_printf(Console_Default, "Available plugins:\n");
    while (ptr->name != NULL)
    {
        console_printf(Console_Default, "%d - %s\n", i, ptr->name);
        ptr++;
        i++;
    }
}

int plugin_load(int id)
{
    Plugin *ptr = &(Plugins[0]);
    int i = id;

    //console_printf(Console_Default, "%s(%d)", __func__, id);

    for (; i > 1 && ptr->name != NULL ; i--)
    {
        //console_printf(Console_Default, "%d - %s\n", i, ptr->name);
        ptr++;
    }

    if (ptr == NULL)
    {
        return -1;
    }

    return ptr->init();
}

int plugin_unload(int id)
{
    Plugin *ptr = &(Plugins[0]);

    for (; id == 0 && ptr != NULL ; id--)
    {
        ptr++;
    }

    if (ptr == NULL)
    {
        return -1;
    }

    return ptr->deinit();
}


/* Available functions for plugins */
int plugin_install_keypressHandler(uint8_t key, PluginKeypress func)
{
    KeyHandler *ptr;

    if (keyHandlersList == NULL)
    {
        keyHandlersList = (KeyHandler *)malloc(sizeof(KeyHandler));

        keyHandlersList->key = key;
        keyHandlersList->func = func;
        keyHandlersList->next = NULL;
    }
    else
    {
        ptr = keyHandlersList;

        while (ptr->next != NULL)
        {
            ptr = ptr->next;
        }

        ptr->next = (KeyHandler *)malloc(sizeof(KeyHandler));

        ptr = ptr->next;

        ptr->key = key;
        ptr->func = func;
        ptr->next = NULL;

    }

    return 0;
}

int plugin_remove_keypressHandler(uint8_t key, PluginKeypress func)
{   /* actually do nothing, we cant remove plugin online */
    return 0;
}


/* Available functions outside of plugins */
int plugin_keypress()
{
    KeyHandler *ptr = keyHandlersList;

    while (ptr != NULL)
    {
        if (getKeyStatus(ptr->key))
        {
            console_printf(Console_Default, "Keyrrr [%d].....\n", ptr->key);
            ptr->func();
        }
        ptr = ptr->next;
    }

    return 0;
}
