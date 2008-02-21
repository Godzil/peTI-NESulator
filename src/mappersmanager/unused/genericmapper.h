/*
 *  Generic mapper implementation - The TI-NESulator Project
 *  genericmapper.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2008 986Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-04-16 01:55:35 +0200 (lun, 16 avr 2007) $
 *  $Author: godzil $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/genericmapper.h $
 *  $Revision: 39 $
 *
 */
 
int _InitMapper(NesCart * cart) 
{
    
    set_prom_bank_16k(0xC000,0);
    set_prom_bank_16k(0x8000,-1);
    
    return 0;
    
} 

int _MapperWriteHook(register word Addr, register byte Value) 
{
    
    if (Addr > 0x7FFF)    /* Try to write to the rom */
    {
        set_vrom_bank_8k(0x0000,Value) 
        
        return 1;
    } 
    return 0;
} 
