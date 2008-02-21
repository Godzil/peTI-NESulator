/*
 *  NOROM Mapper - The TI-NESulator Project
 *  norom.c
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2007 986Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-04-16 01:55:35 +0200 (lun, 16 avr 2007) $
 *  $Author: godzil $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/unrom.h $
 *  $Revision: 39 $
 *
 */

#include "norom.h"

int norom_InitMapper(NesCart *cart) 
{
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