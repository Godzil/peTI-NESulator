/*
 *  norom.c
 *  TI-NESulator.X
 *
 *  Created by Manoël Trapier on 25/10/07.
 *  Copyright 2007 986 Corp. All rights reserved.
 *
 */

#include "norom.h"

int norom_InitMapper(NesCart *cart) 
{
   int i;
      
   set_page_ptr_16k(0x80, cart->PROMBanks); 
   
   /* mUBank = 0xC000 */   
   if (cart->PROMSize > (16*1024))
   {
      set_prom_bank_16k(0xC000, 1);
   }
   else        
   {
      set_prom_bank_16k(0xC000, 0);
   }
   
   if (cart->VROMSize > 0)
      set_vrom_bank_8k(0x2000, 0);
   
   return 0;
}

int norom_MapperIRQ(int cycledone)
{
   return 0;
}

void norom_MapperWriteHook(register byte Addr, register byte Value)
{
   /* Nothing to do */
   return;
}

void norom_MapperDump (FILE *fp)
{
   fprintf(fp, "norom mapper have nothing to dump");
}