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
    
    uint8_t Bit;
    
    uint8_t LastWrite;
    
} Paddle;


uint8_t ReadPaddle(Paddle * pdl);


void InitPaddle(Paddle * pdl);

void WritePaddle(Paddle * pdl, uint8_t val);


#endif
