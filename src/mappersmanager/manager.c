/*
 *  Mapper manager - The TI-NESulator Project
 *  manager.c
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <mappers/manager.h>

#include <os_dependent.h>

MapperIRQ       mapper_irqloop;
MapperDump      mapper_dump;
MapperWriteHook mapper_hook;

typedef struct Mapper_
{   
   byte id;
   char *name;

   MapperInit      init;
   MapperIRQ       irq;
   MapperDump      dump;

} Mapper;

#include "mappers_list.h"

void mapper_list ()
{
   Mapper *ptr = &(Mappers[0]);
   console_printf(Console_Default, "Available mapers:\n");
   while(ptr->name != NULL)
   {
      console_printf(Console_Default, "%d - %s\n", ptr->id, ptr->name);
      ptr++;
   }
}

int mapper_init (NesCart *cart)
{
   Mapper *ptr = &(Mappers[0]);
   console_printf (Console_Default, "Search for a compatible mapper ID #%d:\n", cart->MapperID);
   while (ptr->name != NULL)
   {
      if (ptr->id == cart->MapperID)
      {
         console_printf (Console_Default, "Found mapper ID #%d - '%s'\n", ptr->id, ptr->name);
         if (ptr->init)
            ptr->init (cart);
         
         mapper_irqloop = ptr->irq;
         mapper_dump    = ptr->dump;
         
         return 0;
      }
      ptr++;
   }
   console_printf (Console_Default, "No compatible mapper found!\n");
   return -1;
}
