/*
 *  Paddle manager - The peTI-NESulator Project
 *  paddle.c
 *
 *  Created by Manoël Trapier.
 *  Copyright (c) 2002-2019 986-Studio.
 *
 */
#include <stdio.h>
#include <os_dependent.h>

#include "paddle.h"

void InitPaddle(Paddle *pdl)
{
    pdl->bitPos = 1;
    pdl->strobeState = 0;
}


void WritePaddle(Paddle *pdl, uint8_t val)
{
    pdl->strobeState = val & 1;
    pdl->bitPos = 1;
}

uint8_t ReadPaddle(Paddle *pdl)
{
    uint8_t ret = 0x40;

    switch (pdl->bitPos)
    {
    case 1:
        if (getKeyStatus('O'))
        {
            ret = 0x41;
        }
        break;

    case 2:
        if (getKeyStatus('P'))
        {
            ret = 0x41;
        }
        break;

    case 3:
        if (getKeyStatus('I'))
        {
            ret = 0x41;
        }
        break;

    case 4:
        if (getKeyStatus('U'))
        {
            ret = 0x41;
        }
        break;

    case 5:
        if (getKeyStatus('W'))
        {
            ret = 0x41;
        }
        break;

    case 6:
        if (getKeyStatus('S'))
        {
            ret = 0x41;
        }
        break;

    case 7:
        if (getKeyStatus('A'))
        {
            ret = 0x41;
        }
        break;

    case 8:
        if (getKeyStatus('D'))
        {
            ret = 0x41;
        }
        break;

    default:
        ret = 0x41;
    }

    if ((pdl->strobeState == 0) && (pdl->bitPos <= 9))
    {
        pdl->bitPos++;
    }

    return ret;
} 
