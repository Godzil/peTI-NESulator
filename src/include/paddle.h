/*
 *  Paddle manager - The TI-NESulator Project
 *  paddle.h
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
