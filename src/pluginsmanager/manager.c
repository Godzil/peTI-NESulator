/*
 *  Plugins manager - The TI-NESulator Project
 *  plugins.c
 *
 *  Created by Manoel TRAPIER on 02/04/07.
 *  Copyright (c) 2003-2008 986Corp. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <plugins/manager.h>

typedef struct Plugin_
{   
    char *name;
    
    PluginInit init;
    PluginDeinit deinit;

} Plugin;

typedef struct KeyHandler_
{
    byte key;
    
    PluginKeypress func;
    
    struct KeyHandler_ *next;
    
} KeyHandler;

KeyHandler *keyHandlersList = NULL;

#include "plugins_list.h"

void plugin_list()
{
    Plugin *ptr = &(Plugins[0]);
    int i = 1;
    printf("Available plugins:\n");
    while(ptr->name != NULL)
    {
        printf("%d - %s\n", i, ptr->name);
        ptr++; i++;
    }
}

int plugin_load(int id)
{
    Plugin *ptr = &(Plugins[0]);
    int i = id;
    
    printf("%s(%d)", __func__, id);
    
    for ( ; i > 1 && ptr->name != NULL; i -- )
    {
        printf("%d - %s\n", i, ptr->name);
        ptr ++;        
    }

    if (ptr == NULL)
        return -1;

    return ptr->init();
}

int plugin_unload(int id)
{
    Plugin *ptr = &(Plugins[0]);
    
    for ( ; id == 0 && ptr != NULL; id -- )
        ptr ++;

    if (ptr == NULL)
        return -1;

    return ptr->deinit();
}


/* Available functions for plugins */
int plugin_install_keypressHandler(byte key, PluginKeypress func)
{
    KeyHandler *ptr;
    
    if (keyHandlersList == NULL)
    {
        keyHandlersList = (KeyHandler*) malloc(sizeof(KeyHandler));
        
        keyHandlersList->key = key;
        keyHandlersList->func = func;
        keyHandlersList->next = NULL;
    }
    else
    { 
        ptr = keyHandlersList;
        
        while(ptr->next != NULL)
            ptr = ptr->next;
        
        ptr->next = (KeyHandler*) malloc(sizeof(KeyHandler));
        
        ptr = ptr->next;
        
        ptr->key = key;
        ptr->func = func;
        ptr->next = NULL;

    }
    
    return 0;
}

int plugin_remove_keypressHandler(byte key, PluginKeypress func)
{   /* actually do nothing, we cant remove plugin online */
    return 0;
}


/* Available functions outside of plugins */
int plugin_keypress(byte key)
{
    KeyHandler *ptr = keyHandlersList;
    
    while(ptr != NULL)
    {
        if (ptr->key == key)
        {
            ptr->func();
        }
        ptr = ptr->next;
    }
    
    return 0;
}
