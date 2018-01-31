/*
 *  Paddle manager - The peTI-NESulator Project
 *  paddle.h
 *
 *  Created by Manoel TRAPIER.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
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
