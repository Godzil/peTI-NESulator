/*
 *  Cart manager - The TI-NESulator Project
 *  NESCart.c
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2008 986Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-05-02 18:37:41 +0200 (mer, 02 mai 2007) $
 *  $Author: mtrapier $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/NESCarts.c $
 *  $Revision: 50 $
 *
 */
 
#include "include/NESCarts.h"
#include "include/mappers/manager.h"

#include <stdlib.h>
#include <stdio.h>

/* Plateform dependent function */
void *LoadFilePtr(char * filename);

void DumpCartProperties(FILE *out, NesCart * cart)
{
    fprintf(out,
            "'%s' informations:\n"
            "   Total ROM Size       : 0x%06X    | Total VROM Size      : 0x%06X\n"
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
	byte buffer[6];
    /* Load the cart into memory */
    cart->File = (byte *)LoadFilePtr(filename);
    
    
    if (cart->File == -1)
      return -1;
    
	sprintf(buffer, "%c%c%c%c", 0x4E, 0x45, 0x53, 0x1A);
	
	/* Verify that this is a real iNES valid file */
	if (memcmp(cart->File, buffer, 4))
		return -1;
	
    if ((cart->File == NULL) || (cart->File == -1))
        return -1;
    
    /* Before go elsewhere, verify that the header is clean !
           (aka no DiskDude! in it) */
    if (memcmp(cart->File+7, "DiskDude!", 9) == 0)
    {
        printf("\n"
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
    cart->FileName = filename;

    cart->PROMSize = cart->File[4] * 16 * 1024;               /* Size of PROM */
    cart->VROMSize = cart->File[5] *  8 * 1024;               /* Size of VROM */   
    cart->Flags = cart->File[6] & 0x0F;
    
    /* We don't and we will never support trainer-ed ROM */
    if (cart->Flags & iNES_TRAINER)
    {
        printf("\n"
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
