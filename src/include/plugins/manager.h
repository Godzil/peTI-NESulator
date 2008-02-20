/*
 *  Plugins manager - The TI-NESulator Project
 *  plugins.h
 *
 *  Created by Manoel TRAPIER on 02/04/07.
 *  Copyright (c) 2003-2007 986Corp. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

#ifndef PLUGINS_H 
#define PLUGINS_H 

#include <types.h>

/* Function pointer for prototyping function that plugins may export */
typedef int (*PluginInit) (void);
typedef int (*PluginDeinit) (void);
typedef void (*PluginKeypress) (void);

#ifdef __TINES_PLUGINS__

/* Available functions for plugins */
int plugin_install_keypressHandler(byte key, PluginKeypress);
int plugin_remove_keypressHandler(byte key, PluginKeypress);

#else /* __TINES_PLUGINS__ */

/* Available functions outside of plugins */
int plugin_keypress(byte key);

/* Real Prototype: TBD */
void plugin_list();

int plugin_load(int id);

int plugin_unload(int id);


#endif /* __TINES_PLUGINS__ */

#endif /* PLUGINS_H */
