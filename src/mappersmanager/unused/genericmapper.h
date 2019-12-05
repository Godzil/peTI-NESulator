/*
 *  Generic mapper implementation - The peTI-NESulator Project
 *  genericmapper.h
 *
 *  Created by Manoël Trapier.
 *  Copyright (c) 2002-2019 986-Studio.
 *
 */
 
int _InitMapper(NesCart * cart) 
{
    
    set_prom_bank_16k(0xC000,0);
    set_prom_bank_16k(0x8000,-1);
    
    return 0;
    
} 

int _MapperWriteHook(register word Addr, register uint8_t Value)
{
    
    if (Addr > 0x7FFF)    /* Try to write to the rom */
    {
        set_vrom_bank_8k(0x0000,Value);
        
        return 1;
    } 
    return 0;
} 
