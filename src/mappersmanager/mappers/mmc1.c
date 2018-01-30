/*
 *  MMC1 Mapper - The TI-NESulator Project
 *  mmc1.h
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

#include "mmc1.h"

uint8_t MMC1_reg0;

uint8_t MMC1_reg1;

uint8_t MMC1_reg2;

uint8_t MMC1_reg3;

uint8_t mmc1_CurrentBank;

#define MMC1_R0_MIRROR    0x01
#define MMC1_R0_ONESCREEN  0x02
#define MMC1_R0_PRGAREA   0x04
#define MMC1_R0_PRGSIZE   0x08
#define MMC1_R0_VROMSW    0x10
#define MMC1_R0_RESET     0x80

#define MMC1_R1_VROMB1    0x0F
#define MMC1_R1_256KSEL   0x10
#define MMC1_R1_RESET     0x80

#define MMC1_R2_VROMB2    0x0F
#define MMC1_R2_256KSEL   0x10
#define MMC1_R2_RESET     0x80

#define MMC1_R3_VROMB2    0x0F
#define MMC1_R3_SAVECE    0x10
#define MMC1_R3_RESET     0x80


#define MMC1_REG0_DEFAULT MMC1_R0_PRGSIZE | MMC1_R0_PRGAREA
#define MMC1_REG1_DEFAULT 0
#define MMC1_REG2_DEFAULT 0
#define MMC1_REG3_DEFAULT 0

void mmc1_MapperWriteReg0(register byte Addr, register byte Value);
void mmc1_MapperWriteReg1(register byte Addr, register byte Value);
void mmc1_MapperWriteReg2(register byte Addr, register byte Value);
void mmc1_MapperWriteReg3(register byte Addr, register byte Value);

void mmc1_MapperDump(FILE *fp)
{
    fprintf(fp,"MMC1: r0:0x%02X r1:0x%02X r2:0x%02X r3:0x%02X\n",MMC1_reg0,MMC1_reg1,MMC1_reg2,MMC1_reg3);
}

int mmc1_InitMapper(NesCart * cart) 
{
    int i;
    
       MMC1_reg0 = MMC1_REG0_DEFAULT;
       
       MMC1_reg1 = MMC1_REG1_DEFAULT;
       
       MMC1_reg2 = MMC1_REG2_DEFAULT;
       
       MMC1_reg3 = MMC1_REG3_DEFAULT;
        
       set_prom_bank_16k(0x8000,0);
       set_prom_bank_16k(0xC000, GETLAST16KBANK(cart));
       
       mmc1_CurrentBank = 0;
       
       if (cart->VROMSize > 0)
           set_vrom_bank_4k(0x0000,0);

       
       /* Mapper should register itself for write hook */
    for (i = 0x80; i < 0xA0 ; i++)
    {
       set_page_wr_hook(i, mmc1_MapperWriteReg0);
       set_page_writeable(i, true);       
    } 
    for (i = 0xA0; i < 0xC0 ; i++)
    {
       set_page_wr_hook(i, mmc1_MapperWriteReg1);
       set_page_writeable(i, true);       
    } 
    for (i = 0xC0; i < 0xE0 ; i++)
    {
       set_page_wr_hook(i, mmc1_MapperWriteReg2);
       set_page_writeable(i, true);       
    } 
    for (i = 0xE0; i < 0x100 ; i++)
    {
       set_page_wr_hook(i, mmc1_MapperWriteReg3);
       set_page_writeable(i, true);       
    }
    
    return 0;
       
} 


/*
Reg 0
8 : 1000
9 : 1001

        Reg 1
        A : 1010
        B : 1011
        
          Reg 2
          C : 1100
          D : 1101
          
            Reg 3
            E : 1110
            F : 1111
            
              ((Addr & 0x6000) >> 13) <- port number
*/ 
#define MMC1_GetReg(a) ((a & 0x6000) >> 13)
/* (Val & 0x01) recuperation du bit */ 
#define MMC1_GetBit(v) (v & 0x01)
/* ( ( b & (1 << Bit)) | (v << Bit) ) Ajout du bit */ 
#define MMC1_AddBit(b,v) ( ( b & ~(1 << Bit)) | (v << Bit) )

void mmc1_ApplyReg0Mod() 
{
    
    static uint8_t OldSwitchArea = MMC1_R0_PRGAREA;
    
    
       
    //console_printf(Console_Default, "Change to reg0 done ! (0x%x)\n\tMiror flag : %d\n\tOneScreen Flag : %d\n\tPRG Size : %d\n\tPRG Area : %d\n\tVROM Switch size : %d\n", MMC1_reg0, MMC1_reg0 & MMC1_R0_MIRROR, MMC1_reg0 & MMC1_R0_ONESCREEN, MMC1_reg0 & MMC1_R0_PRGAREA, MMC1_reg0 & MMC1_R0_PRGSIZE, MMC1_reg0 & MMC1_R0_VROMSW);
        
    switch (MMC1_reg0 & 0x03)
    {
    case 0:
        ppu_setScreenMode(PPU_SCMODE_SINGLE);
        ppu_setSingleScreen(PPU_SCREEN_000);
        break;
    case 1:
        ppu_setScreenMode(PPU_SCMODE_SINGLE);
        ppu_setSingleScreen(PPU_SCREEN_400);
        break;
    case 2:
        ppu_setScreenMode(PPU_SCMODE_NORMAL);
        ppu_setMirroring(PPU_MIRROR_VERTICAL);
        break;
    case 3:
        ppu_setScreenMode(PPU_SCMODE_NORMAL);
        ppu_setMirroring(PPU_MIRROR_HORIZTAL);
        break;
    }
       
    if ( (OldSwitchArea != (MMC1_reg0 & MMC1_R0_PRGAREA)) && ((MMC1_reg0 & MMC1_R0_PRGSIZE) != 0 ) )
    {
           
        if ((MMC1_reg0 & MMC1_R0_PRGAREA) != 0)
        {        /* 0x8000 area */
            set_prom_bank_16k(0x8000,mmc1_CurrentBank);
            set_prom_bank_16k(0xC000, GETLAST16KBANK(Cart));
        } 
        else
        {        /* 0xC000 area */
    
            set_prom_bank_16k(0x8000,0);
            set_prom_bank_16k(0xC000,mmc1_CurrentBank);

        } 
           
        OldSwitchArea = (MMC1_reg0 & MMC1_R0_PRGAREA);
    }
       
} 

uint32_t VROMBankNb;
uint8_t Bit = 0;
uint8_t BitBuf = 0;

void mmc1_MapperWriteReg0(register byte Addr, register byte Value) 
{
    if (Value & 0x80) 
    {   
        MMC1_reg0 = MMC1_REG0_DEFAULT;
        console_printf(Console_Default, "MMC1: Reg0 Reset occured !\n");
        mmc1_ApplyReg0Mod();
    }
    else
    {
        if (Bit < 4)
        {        /* Pas encore ecrit les 5 bits  */
            BitBuf = MMC1_AddBit(BitBuf, MMC1_GetBit(Value));
            Bit++;
        } 
        else
        {
            BitBuf = MMC1_AddBit(BitBuf, MMC1_GetBit(Value));
            Bit = 0;
            
            MMC1_reg0 = BitBuf;
            
            mmc1_ApplyReg0Mod();
        }
    }               
}

void mmc1_MapperWriteReg1(register byte Addr, register byte Value) 
{
    if (Value & 0x80) 
    {   
        MMC1_reg1 = MMC1_REG1_DEFAULT;
        console_printf(Console_Default, "MMC1: Reg1 Reset occured !\n");
    }
    else
    {
        if (Bit < 4)
        {        /* Pas encore ecrit les 5 bits  */
            BitBuf = MMC1_AddBit(BitBuf, MMC1_GetBit(Value));
            Bit++;
        } 
        else
        {
            BitBuf = MMC1_AddBit(BitBuf, MMC1_GetBit(Value));
            Bit = 0;
            
            MMC1_reg1 = BitBuf;
                        
            VROMBankNb = (MMC1_reg1 /* & MMC1_R1_VROMB1 */ );

            if (Cart->VROMSize == 0)
            {
                console_printf(Console_Default, "Try to change VROM but with didn't have any VROM ! [%04X]\n", VROMBankNb);
                return;
            }
                
            if ( (MMC1_reg0 & MMC1_R0_VROMSW) != 0 )
            {    /* 4K vram */
                //console_printf(Console_Default, "Switching VROM at 0x0000 to 4k bank %d\n", VROMBankNb);
                set_vrom_bank_4k(0x0000,VROMBankNb);
            }
            else
            {    /* 8K vram */
                //console_printf(Console_Default, "Switching VROM at 0x0000 to 8k bank %d\n", VROMBankNb>>1);
                set_vrom_bank_8k(0x0000,VROMBankNb>>1);
            }
        }
    }
}

void mmc1_MapperWriteReg2(register byte Addr, register byte Value) 
{
    if (Value & 0x80) 
    {   
        MMC1_reg2 = MMC1_REG2_DEFAULT;
        console_printf(Console_Default, "MMC1: Reg2 Reset occured !\n");
    }
    else
    {
        if (Bit < 4)
        {        /* Pas encore ecrit les 5 bits  */
            BitBuf = MMC1_AddBit(BitBuf, MMC1_GetBit(Value));
            Bit++;
        } 
        else
        {
            BitBuf = MMC1_AddBit(BitBuf, MMC1_GetBit(Value));
            Bit = 0;

            MMC1_reg2 = BitBuf;
            
            VROMBankNb = (MMC1_reg2 /* & MMC1_R2_VROMB2 */ );
            
            //console_printf(Console_Default, "Want to switch VROM at 0x1000 to 4k bank %d\n", VROMBankNb);
            if (Cart->VROMSize == 0)
            {
                //console_printf(Console_Default, ": No\n");
                return;
            }
            
            if ( (MMC1_reg0 & MMC1_R0_VROMSW) != 0 )
            {    /* 4K vram */
                //console_printf(Console_Default, "Switching VROM at 0x1000 to 4k bank %d\n", VROMBankNb);
                set_vrom_bank_4k(0x1000,VROMBankNb);
            }
            else
            {    /* 8K vram */
            //       console_printf(Console_Default, "Switching VROM at 0x1000 to 8k bank %d\n", VROMBankNb>>1);
            //       set_vrom_bank_8k(0x1000,VROMBankNb>>1);
            }
        }
    }
}

void mmc1_MapperWriteReg3(register byte Addr, register byte Value) 
{
    if (Value & 0x80) 
    {   
        MMC1_reg3 = MMC1_REG3_DEFAULT;
        console_printf(Console_Default, "MMC1: Reg3 Reset occured !\n");
    }
    else
    {
        if (Bit < 4)
        {        /* Pas encore ecrit les 5 bits  */
            BitBuf = MMC1_AddBit(BitBuf, MMC1_GetBit(Value));
            Bit++;
        } 
        else
        {
            BitBuf = MMC1_AddBit(BitBuf, MMC1_GetBit(Value));
            Bit = 0;
        
            MMC1_reg3 = BitBuf;
            
            if ( ((uint32_t)MMC1_reg3 << 14) > Cart->PROMSize)
                return;
                
            if ( (MMC1_reg0 & MMC1_R0_PRGSIZE) != 0 )
            {    /* 16K Switch */
                if ( (MMC1_reg0 & MMC1_R0_PRGAREA) != 0 )
                {    /* 0x8000 switch */
                    set_prom_bank_16k(0x8000,MMC1_reg3);
                    //console_printf(Console_Default, "LowBank is now %d ( 0x%p )\n", MMC1_reg3, mLBank);
                }
                else
                {    /* 0xC000 switch */
                    set_prom_bank_16k(0xC000,MMC1_reg3);
                    //console_printf(Console_Default, "HighBank is now %d ( 0x%p )\n", MMC1_reg3, mUBank);
                }
            }
            else
            {    /* 32K Switch */
                set_prom_bank_32k(0x8000,MMC1_reg3>>1);
            }
            
            if ( ( MMC1_reg3 & MMC1_R3_SAVECE ) != 0)   
            {
                unmap_sram();
            }
            else
            {
                map_sram();
            }
        }
    }
}    
        
        //console_printf(Console_Default, "MMC1: Debug (Reg:%d,Val:0x%02X,reg0:0x%02X,reg1:0x%02X,reg2:0x%02X,reg3:0x%02X)\n", MMC1_GetReg(Addr), Value, MMC1_reg0, MMC1_reg1, MMC1_reg2, MMC1_reg3);
