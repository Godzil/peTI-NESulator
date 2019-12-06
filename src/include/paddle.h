/*
 *  Paddle manager - The peTI-NESulator Project
 *  paddle.h
 *
 *  Created by Manoël Trapier.
 *  Copyright (c) 2002-2019 986-Studio.
 *
 */

#ifndef PADDLE_H
#define PADDLE_H

typedef struct Paddle_
{
    uint8_t bitPos;
    uint8_t strobeState;
} Paddle;

uint8_t ReadPaddle(Paddle *pdl);
void InitPaddle(Paddle *pdl);
void WritePaddle(Paddle *pdl, uint8_t val);

#endif
