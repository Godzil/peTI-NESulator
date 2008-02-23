/*
 *  Plugin Manager plugint list - The TI-NESulator Project
 *  plugins_list.h
 *
 *  Created by Manoel Trapier.
 *  Copyright 2003-2008 986 Corp. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

/* This file could be generated from the plugins directory... */

#include "plugins/gamegenie.h"

Plugin Plugins[] = {
        { "Game Genie", gg_Init, gg_Deinit },
 
/* EOL tag */
         { NULL, NULL, NULL }
                   };
