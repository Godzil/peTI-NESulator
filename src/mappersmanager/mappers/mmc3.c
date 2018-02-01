/*
 *  MMC3 Mapper - The peTI-NESulator Project
 *  mmc3.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 */
#include "mmc3.h"

extern uint16_t ScanLine;

uint16_t mmc3_command;

uint8_t mmc3_irq_counter;
uint8_t mmc3_irq_counter_reload;
uint8_t mmc3_irq_enable;

uint16_t mmc3_first_prom_page;
uint16_t mmc3_second_prom_page;

uint8_t  mmc3_use_xor;
uint8_t mmc3_last_vrom[6];

uint8_t mmc3_last_prom[2];
uint8_t mmc3_last_prom_switch;

uint16_t dummy;

void mmc3_MapperWrite80Hook(uint8_t addr, uint8_t value);
void mmc3_MapperWriteA0Hook(uint8_t addr, uint8_t value);
void mmc3_MapperWriteC0Hook(uint8_t addr, uint8_t value);
void mmc3_MapperWriteE0Hook(uint8_t addr, uint8_t value);

void mmc3_MapperDump(FILE *fp)
{
    fprintf(fp,"MMC3: CMD:%d IC:%d IR:%d IE:%d FPP:0x%04X SPP:0x%04X UX:%d\n",mmc3_command,mmc3_irq_counter,mmc3_irq_counter_reload,mmc3_irq_enable,mmc3_first_prom_page,mmc3_second_prom_page,mmc3_use_xor);
    fprintf(fp,"MMC3: LV0:%d LV1:%d LV2:%d LV3:%d LV4:%d LV5:%d\n",mmc3_last_vrom[0],mmc3_last_vrom[1],mmc3_last_vrom[2],mmc3_last_vrom[3],mmc3_last_vrom[4],mmc3_last_vrom[5]);
    fprintf(fp,"MMC3: LP0:%d LP1:%d LPS:%d\n",mmc3_last_prom[0],mmc3_last_prom[1],mmc3_last_prom_switch);
}

int mmc3_InitMapper(NesCart * cart) 
{
    set_prom_bank_16k(0x8000, 0);
    set_prom_bank_16k(0xC000, GETLAST16KBANK(cart));

    if ( Cart->VROMSize > 0)
    {
         set_vrom_bank_8k(0, 0x0000);
    }
    
    mmc3_command = -1;

    mmc3_irq_counter = -1;
    mmc3_irq_enable = 0;
    mmc3_irq_counter_reload = 0;
    
    mmc3_use_xor = 0x42;
    
    mmc3_last_prom_switch = 0x42;
    
    mmc3_last_prom[0] = 0;
    mmc3_last_prom[1] = 1;
    
    mmc3_last_vrom[0] = 0;
    mmc3_last_vrom[1] = 2;
    mmc3_last_vrom[2] = 3;
    mmc3_last_vrom[3] = 4;
    mmc3_last_vrom[4] = 5;
    mmc3_last_vrom[5] = 6;
    
    mmc3_first_prom_page = 0x8000;
    mmc3_second_prom_page = 0xA000; 
    
    /* Register mapper write hook */
    set_page_wr_hook(0x80, mmc3_MapperWrite80Hook);
    set_page_writeable(0x80, true);
    
    set_page_wr_hook(0xA0, mmc3_MapperWriteA0Hook);
    set_page_writeable(0xA0, true);
    
    set_page_wr_hook(0xC0, mmc3_MapperWriteC0Hook);
    set_page_writeable(0xC0, true);
    
    set_page_wr_hook(0xE0, mmc3_MapperWriteE0Hook);
    set_page_writeable(0xE0, true);

    return 0;
} 

void mmc3_MapperWrite80Hook(uint8_t addr, uint8_t Value)
{
    //console_printf(Console_Default, "%s(0x%02X, 0x%02X)\n", __func__, addr, Value);
    if (addr > 0x01)
        return;

    if (!addr)
    {

        if ((Cart->VROMSize > 0) && ( mmc3_use_xor != (Value & 0x80) ))
        {
    		if (Value & 0x80)
    		{
    			set_vrom_bank_1k(0x0000, mmc3_last_vrom[2]);
    			set_vrom_bank_1k(0x0400, mmc3_last_vrom[3]);
    			set_vrom_bank_1k(0x0800, mmc3_last_vrom[4]);
    			set_vrom_bank_1k(0x0C00, mmc3_last_vrom[5]);
    			set_vrom_bank_2k(0x1000, mmc3_last_vrom[0]>>1);
    			set_vrom_bank_2k(0x1800, mmc3_last_vrom[1]>>1);
    			//chr4,chr5,chr6,chr7,chr01,chr01+1,chr23,chr23+1
    		}
    		else
    		{
    		    set_vrom_bank_2k(0x0000, mmc3_last_vrom[0]>>1);
    			set_vrom_bank_2k(0x0800, mmc3_last_vrom[1]>>1);
    			set_vrom_bank_1k(0x1000, mmc3_last_vrom[2]);
    			set_vrom_bank_1k(0x1400, mmc3_last_vrom[3]);
    			set_vrom_bank_1k(0x1800, mmc3_last_vrom[4]);
    			set_vrom_bank_1k(0x1C00, mmc3_last_vrom[5]);
    			//chr01,chr01+1,chr23,chr23+1,chr4,chr5,chr6,chr7
    		}
   			mmc3_use_xor = (Value & 0x80);
    	}
    	
		
		if (mmc3_last_prom_switch != (Value & 0x40))
		{
    		if (!(Value & 0x40))
    		{
              console_printf(Console_Default, "MMC3: Switch -> 8/A\n");
    		    mmc3_first_prom_page = 0x8000;
    		    mmc3_second_prom_page = 0xA000;
    		    
    		    set_prom_bank_8k(mmc3_first_prom_page, mmc3_last_prom[0]);
    		    set_prom_bank_8k(mmc3_second_prom_page, mmc3_last_prom[1]);
    		    
    		    set_prom_bank_8k(0xC000, GETLAST08KBANK(Cart)-1);
    		    //set_prom_bank_8k(0xE000, GETLAST08KBANK(cart));
    			//prg_bank(prg0,prg1,max_prg-1,max_prg);
    		}
    		else
    		{
    		    console_printf(Console_Default, "MMC3: Switch -> C/A\n");
    		    mmc3_first_prom_page = 0xC000;
    		    mmc3_second_prom_page = 0xA000;
    		
    		    set_prom_bank_8k(mmc3_first_prom_page, mmc3_last_prom[0]);
    		    set_prom_bank_8k(mmc3_second_prom_page, mmc3_last_prom[1]);
    		    
    		    set_prom_bank_8k(0x8000, GETLAST08KBANK(Cart)-1);
    
        	
    			//prg_bank(max_prg-1,prg1,prg0,max_prg);
    		}
          mmc3_last_prom_switch = (Value & 0x40);	
    	}
     mmc3_command = Value & 0x07;
		
		

    } else { /* 8001 */
		switch (mmc3_command)
		{
		case 0: 
		case 1: 
		case 2:
		case 3: 
		case 4: 
		case 5:  
		    if (Cart->VROMSize == 0)
		        return;

		    mmc3_last_vrom[mmc3_command] = Value;
		    
		    if (mmc3_use_xor)
    		{
    			set_vrom_bank_1k(0x0000, mmc3_last_vrom[2]);
    			set_vrom_bank_1k(0x0400, mmc3_last_vrom[3]);
    			set_vrom_bank_1k(0x0800, mmc3_last_vrom[4]);
    			set_vrom_bank_1k(0x0C00, mmc3_last_vrom[5]);
    			set_vrom_bank_2k(0x1000, mmc3_last_vrom[0]>>1);
    			set_vrom_bank_2k(0x1800, mmc3_last_vrom[1]>>1);
    			//chr4,chr5,chr6,chr7,chr01,chr01+1,chr23,chr23+1
    		}
    		else
    		{
    		    set_vrom_bank_2k(0x0000, mmc3_last_vrom[0]>>1);
    			set_vrom_bank_2k(0x0800, mmc3_last_vrom[1]>>1);
    			set_vrom_bank_1k(0x1000, mmc3_last_vrom[2]);
    			set_vrom_bank_1k(0x1400, mmc3_last_vrom[3]);
    			set_vrom_bank_1k(0x1800, mmc3_last_vrom[4]);
    			set_vrom_bank_1k(0x1C00, mmc3_last_vrom[5]);
    			//chr01,chr01+1,chr23,chr23+1,chr4,chr5,chr6,chr7
    		}
		    
			break;
			
		case 6: 
		    mmc3_last_prom[0] = Value;
            set_prom_bank_8k(mmc3_first_prom_page, mmc3_last_prom[0]);
			break;
			
		case 7:
		    mmc3_last_prom[1] = Value;
            set_prom_bank_8k(mmc3_second_prom_page, mmc3_last_prom[1]);
			break;
			
		}
		
        /*if(mmc3_use_xor)
    	    chr_bank(chr4,chr5,chr6,chr7,chr01,chr01+1,chr23,chr23+1);
        else
    		chr_bank(chr01,chr01+1,chr23,chr23+1,chr4,chr5,chr6,chr7);*/
    }


}

void mmc3_MapperWriteA0Hook(uint8_t addr, uint8_t Value)
{
    //console_printf(Console_Default, "%s(0x%02X, 0x%02X)\n", __func__, addr, Value);
    if (addr > 0x01)
        return;
    
    if (!addr)    
    {    
      //console_printf(Console_Default, "MMC3: Select mirroring (0xA000) : 0x%X\n",Value);
      
      if (Value & 0x1)
        ppu_setMirroring(PPU_MIRROR_HORIZTAL);
      else
        ppu_setMirroring(PPU_MIRROR_VERTICAL);

    } 
    else
    {
      //console_printf(Console_Default, "MMC3: SaveRAM Toggle (0xA001) : 0x%X\n",Value);
      if (Value)
          map_sram();
      else
          unmap_sram();
    }
      
}

void mmc3_MapperWriteC0Hook(uint8_t addr, uint8_t Value)
{
    //console_printf(Console_Default, "%s(0x%02X, 0x%02X)\n", __func__, addr, Value);
    if (addr > 0x01)
        return;
        
    if (!addr)
    {
     mmc3_irq_counter_reload = Value;
     mmc3_irq_counter = Value;
     //console_printf(Console_Default, "MMC3 IRQ[%d]: SetIRQ reload to %d\n", ScanLine, Value);
     
    }else{ /* C001 */
      //console_printf(Console_Default, "MMC3: New tmp IRQ value (0xC001) : 0x%X\n",Value);
      //console_printf(Console_Default, "MMC3 IRQ[%d]: Reset IRQ counter to val %d [Value = %d]\n", ScanLine, mmc3_irq_counter_reload, Value);
      mmc3_irq_counter = Value;
    }
}

void mmc3_MapperWriteE0Hook(uint8_t addr, uint8_t Value)
{
    //console_printf(Console_Default, "%s(0x%02X, 0x%02X)\n", __func__, addr, Value);
    if (addr > 0x01)
        return;
        
    if (!addr)
    {
      //console_printf(Console_Default, "MMC3: Writing to 0xE001 : 0x%X\n",Value);
      //console_printf(Console_Default, "MMC3 IRQ[%d]: IRQ disabled\n", ScanLine);
      mmc3_irq_enable = 0;
      //MapperWantIRQ = 1;
      // Add a way to raise an IRQ
      
     }else{ /* E001 */
      //console_printf(Console_Default, "MMC3: Writing to 0xE001 : 0x%X\n",Value);
      //console_printf(Console_Default, "MMC3: IRQ Enabled (value : %d)\n",mmc3_irq_counter);
      //console_printf(Console_Default, "MMC3 IRQ[%d]: Enable IRQ\nr", ScanLine);
      mmc3_irq_enable = 1;
    }
}

int mmc3_MapperIRQ(int cycledone)
{
 if (((cycledone > 0) && (cycledone < 241)) /*&&
      (ppu.ControlRegister2.b & (PPU_CR2_BGVISIBILITY | PPU_CR2_SPRTVISIBILITY)) == (PPU_CR2_BGVISIBILITY | PPU_CR2_SPRTVISIBILITY)*/)
  {

    if ((mmc3_irq_counter --) > 0 )return 0;   
    
    
    /* Load next counter position */
    mmc3_irq_counter = mmc3_irq_counter_reload;
    
    if (mmc3_irq_enable == 0) return 0;
    
    mmc3_irq_enable = 0;
    
    //console_printf(Console_Default, "MMC3 IRQ[%d]: Tick next at %d\n", ScanLine, mmc3_irq_counter_reload);
    
    return 1;
  }
  return 0;
}
