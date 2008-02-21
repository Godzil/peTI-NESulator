/*
 *  Paddle manager - The TI-NESulator Project
 *  paddle.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2008 986Corp. All rights reserved.
 *
 *  $LastChangedDate: 2007-04-16 01:55:35 +0200 (lun, 16 avr 2007) $
 *  $Author: godzil $
 *  $HeadURL: file:///media/HD6G/SVNROOT/trunk/TI-NESulator/src/paddle.h $
 *  $Revision: 39 $
 *
 */

#ifndef PADDLE_H
#define PADDLE_H

typedef struct Paddle_ 
{
    
    unsigned char Bit;
    
    unsigned char LastWrite;
    
} Paddle;


unsigned char ReadPaddle(Paddle * pdl);


void InitPaddle(Paddle * pdl);

void WritePaddle(Paddle * pdl, unsigned char val);


#endif
