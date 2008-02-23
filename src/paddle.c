/*
 *  Paddle manager - The TI-NESulator Project
 *  paddle.c
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2008 986Corp. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

#include <allegro.h>
#include "paddle.h"

void InitPaddle(Paddle * pdl) 
{
    pdl->Bit = 1;
    pdl->LastWrite = 0;
} 


void WritePaddle(Paddle *pdl, unsigned char val) 
{
    if ((pdl->LastWrite == 1) && (val == 0))
        InitPaddle(pdl);
    
       pdl->LastWrite = val;
} 
unsigned char ReadPaddle(Paddle * pdl) 
{
    switch (pdl->Bit++)    
    {
        
        case 1:
        if (key[KEY_Z])
            
            return 0x41;
        
           break;
        
    case 2:
        
           if (key[KEY_X])
            
               return 0x41;
        
           break;
        
    case 3:
        
           if (key[KEY_P])
            
               return 0x41;
        
           break;
        
    case 4:
        
           if (key[KEY_ENTER])
            
               return 0x41;
        
           break;
        
    case 5:
        
           if (key[KEY_UP])
            
               return 0x41;
        
           break;
        
    case 6:
        
           if (key[KEY_DOWN])
            
               return 0x41;
        
           break;
        
    case 7:
        
           if (key[KEY_LEFT])
            
               return 0x41;
        
           break;
        
    case 8:
        
           if (key[KEY_RIGHT])
            
               return 0x41;
        
           break;
        
    case 20:
        
           return 0x41;
        
           break;
        
    case 24:
        
           pdl->Bit = 1;
        
           return 0x40;
        
    default:
        
           return 0x40;
        
           break;
        
    } 
       return 0x40;
    
} 
