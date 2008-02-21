/*
 *  manager.c
 *  TI-NESulator.X
 *
 *  Created by Manoël Trapier on 07/10/07.
 *  Copyright 2007 986 Corp. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <mappers/manager.h>

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
   printf("Available mapers:\n");
   while(ptr->name != NULL)
   {
      printf("%d - %s\n", ptr->id, ptr->name);
      ptr++;
   }
}

int mapper_init (NesCart *cart)
{
   Mapper *ptr = &(Mappers[0]);
   printf ("Search for a compatible mapper ID #%X:\n", cart->MapperID);
   while (ptr->name != NULL)
   {
      if (ptr->id == cart->MapperID)
      {
         printf ("Found mapper ID #%X - '%s'\n", ptr->id, ptr->name);
         ptr->init (cart);
         
         mapper_irqloop = ptr->irq;
         mapper_dump    = ptr->dump;
         
         return 0;
      }
      ptr++;
   }
   return -1;
}