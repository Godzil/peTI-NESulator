/*
 *  Cart manager - The peTI-NESulator Project
 *  NESCart.c
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

/* System Headers */
#if !defined(__TIGCC__) && !defined(__GCC4TI__) && !defined(__GTC__)
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#else /* Support for TI-68k compilation */

#define TIGCC_COMPAT
#include <tigcclib.h>
#endif

/* peTI-NESulator headers */
#include <os_dependent.h>
#include <NESCarts.h>
#include <os_dependent.h>
#include <mappers/manager.h>
#include <sys/mman.h>

void DumpCartProperties(FILE *out, NesCart * cart)
{
    console_printf(Console_Verbose,
            "'%s' informations:\n"
            "   Total ROM Size       : 0x%06lX    | Total VROM Size      : 0x%06lX\n"
            "   Mapper ID            : 0x%06X    | Mirroring ?          : %s\n"
            "   Battery ?            : %s         | 4 Screen ?           : %s   \n"
            "   PROMBanks start at   : %p  |\n"
            "   VROMBanks start at   : %p  |\n",
            cart->FileName,
            cart->PROMSize,
            cart->VROMSize,
            cart->MapperID,
            cart->Flags & iNES_MIRROR?  "Horizontal" : "Vertical",
            cart->Flags & iNES_BATTERY? "Yes": "No ",
            cart->Flags & iNES_4SCREEN? "Yes": "No ",
            cart->PROMBanks,
            cart->VROMBanks);
}

int LoadCart(const char *filename, NesCart * cart) 
{
    char buffer[6];
    /* Load the cart into memory */
    cart->File = (byte *)LoadFilePtr((char *)filename);
    
    
    if ((cart->File == NULL) || (cart->File == MAP_FAILED))
      return -1;
    
	sprintf(buffer, "%c%c%c%c", 0x4E, 0x45, 0x53, 0x1A);
	
	/* Verify that this is a real iNES valid file */
	if (memcmp(cart->File, buffer, 4))
		return -1;
    
    /* Before go elsewhere, verify that the header is clean !
           (aka no DiskDude! in it) */
    if (memcmp(cart->File+7, "DiskDude!", 9) == 0)
    {
        console_printf(Console_Warning, "\n"
               "*******************WARNING****************\n"
               "* The header of this game is not clean   *\n"
               "* (DiskDude! pollution) I will only use  *\n"
               "* basic MapperID (mapper 0-15). This can *\n"
               "* led to unexpected behavior...          *\n"
               "*                                        *\n"
               "*        PLEASE CLEAN THIS FILE!         *\n"
               "******************WARNING*****************\n\n");
        /* So this rom file is not clean, we can only rely on the "basic" mapperID */
        cart->MapperID = cart->File[6]>>4;  /* Mapper Type */

    }
    else
    {   /* This rom file is clean, we can read the extended MapperID */
        cart->MapperID = (cart->File[6]>>4)|(cart->File[7]&0xF0);  /* Mapper Type */
    }

    /* Now fill the structure */
    cart->FileName = (char *)filename;

    cart->PROMSize = cart->File[4] * 16 * 1024;               /* Size of PROM */
    cart->VROMSize = cart->File[5] *  8 * 1024;               /* Size of VROM */   
    cart->Flags = cart->File[6] & 0x0F;
    
    /* We don't and we will never support trainer-ed ROM */
    if (cart->Flags & iNES_TRAINER)
    {
        console_printf(Console_Error, "\n"
               "********************ERROR*****************\n"
               "* This cart have an embedded trainer.    *\n"
               "* There is NO support for them.          *\n"
               "* Please use a CLEAN dump if you want    *\n"
               "* to play this game.                     *\n"
               "********************ERROR*****************\n\n");
        return -1; 
    }
    
    cart->PROMBanks = cart->File + 16;                     /* Pointer on the first PROM */
    
    cart->VROMBanks = cart->PROMBanks + cart->PROMSize;    /* Pointer on the first VROM */       

    DumpCartProperties(stdout, cart);

    return 0;    
} 
