/*
 *  IREMH3001 Mapper - The TI-NESulator Project
 *  iremh3001.c
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2007 986Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-04-16 01:55:35 +0200 (lun, 16 avr 2007) $
 *  $Author: godzil $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/iremh3001.h $
 *  $Revision: 39 $
 *
 */

#include "iremh3001.h"

unsigned short iremh3001_prom_slot[3];

unsigned short iremh3001_vrom_slot[8];

int iremh3001_InitMapper(NesCart * cart) 
{
    
    set_prom_bank_16k(0x8000, 0);
    set_prom_bank_16k(0xC000, GETLAST16KBANK(cart));
    
    iremh3001_prom_slot[0] = 0;
    iremh3001_prom_slot[1] = 1;
    iremh3001_prom_slot[2] = GETLAST16KBANK(cart);

    set_vrom_bank_8k(0x0000,4);

    iremh3001_vrom_slot[0] = 0;
    iremh3001_vrom_slot[1] = 0;
    iremh3001_vrom_slot[2] = 0;
    iremh3001_vrom_slot[3] = 0;
    iremh3001_vrom_slot[4] = 0;
    iremh3001_vrom_slot[5] = 0;
    iremh3001_vrom_slot[6] = 0;
    iremh3001_vrom_slot[7] = 0;

    return 0;
    
} 

int iremh3001_MapperWriteHook(register byte Addr, register byte Value) 
{

  switch(Addr)
  {
  case 0x8000: /* Set 8k PROM @ 8000 */
    printf("iremh3001: %X: change PROM to %d[%X]\n", Addr, Value, Value);
    set_prom_bank_8k(0x8000, Value);
    iremh3001_prom_slot[0] = Value;
    break;

  case 0x9003: /* Mirroring ??? */
    printf("iremh3001: Mirroring[0x%X:%d] ?\n", Value, Value);
    break;

  case 0x9005: /* IRQ ??? */
    printf("iremh3001: IRQ[0x%X:%d] ?\n", Value, Value);
    break;

  case 0x9006: /* IRQ ??? */
    printf("iremh3001: IRQ[0x%X:%d] ?\n", Value, Value);
    break;

  case 0xA000: /* Set 8k PROM @ A000 */
    printf("iremh3001: %X: change PROM to %d[%X]\n", Addr, Value, Value);
    set_prom_bank_8k(0xA000, Value);
    iremh3001_prom_slot[1] = Value;
    break;

  case 0xB000: /* Set 1k VROM @ 0000 */
  case 0xB001: /* Set 1k VROM @ 0400 */
  case 0xB002: /* Set 1k VROM @ 0800 */
  case 0xB003: /* Set 1k VROM @ 0C00 */
  case 0xB004: /* Set 1k VROM @ 1000 */
  case 0xB005: /* Set 1k VROM @ 1400 */
  case 0xB006: /* Set 1k VROM @ 1800 */
  case 0xB007: /* Set 1k VROM @ 1C00 */
    printf("iremh3001: %X: change VROM to %d[%X]\n", (Addr&0x0F)<<10, Value, Value);
    set_vrom_bank_1k((Addr&0xF)<<10, Value);
    iremh3001_vrom_slot[Addr&0x0F] = Value;
    break;

  case 0xC000: /* Set 8k PROM @ C000 */
    printf("iremh3001: %X: change PROM to %d[%X]\n", Addr, Value, Value);
    set_prom_bank_8k(0xC000, Value);
    iremh3001_prom_slot[2] = Value;
    break;

  default:
    printf("@:%X -- V:%X", Addr, Value);
    return 0;

  }

  return 1;
} 

void iremh3001_MapperDump(FILE *fp)
{
    fprintf(fp,"iremh3001: prom: $8000:%d $A000:%d $C000:%d\n",
        iremh3001_prom_slot[0], 
        iremh3001_prom_slot[1],
        iremh3001_prom_slot[2]);

    fprintf(fp,"iremh3001: vrom: $0000:%d $0400:%d $0800:%d $0C00:%d\n" \
           "                 $1000:%d $1400:%d $1800:%d $1C00:%d\n",
        iremh3001_vrom_slot[0], 
        iremh3001_vrom_slot[1],
        iremh3001_vrom_slot[2], 
        iremh3001_vrom_slot[3],
        iremh3001_vrom_slot[4], 
        iremh3001_vrom_slot[5],
        iremh3001_vrom_slot[6],
        iremh3001_prom_slot[7]);
}


int iremh3001_MapperIRQ(int cycledone)
{

  return 0;
}
