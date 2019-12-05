/*
 *  Paddle manager - The peTI-NESulator Project
 *  paddle.c
 *
 *  Created by Manoël Trapier.
 *  Copyright (c) 2002-2019 986-Studio.
 *
 */

#include <os_dependent.h>

#include "paddle.h"

void InitPaddle(Paddle *pdl)
{
    pdl->Bit = 1;
    pdl->LastWrite = 0;
}


void WritePaddle(Paddle *pdl, uint8_t val)
{
    if ((pdl->LastWrite == 1) && (val == 0))
    {
        InitPaddle(pdl);
    }

    pdl->LastWrite = val;
}

uint8_t ReadPaddle(Paddle *pdl)
{
    switch (pdl->Bit++)
    {

    case 1:
        if (getKeyStatus('O'))
        {
            return 0x41;
        }
        break;

    case 2:
        if (getKeyStatus('P'))
        {
            return 0x41;
        }
        break;

    case 3:
        if (getKeyStatus('I'))
        {
            return 0x41;
        }
        break;

    case 4:
        if (getKeyStatus('U'))
        {
            return 0x41;
        }
        break;

    case 5:
        if (getKeyStatus('W'))
        {
            return 0x41;
        }
        break;

    case 6:
        if (getKeyStatus('S'))
        {
            return 0x41;
        }
        break;

    case 7:
        if (getKeyStatus('A'))
        {
            return 0x41;
        }
        break;

    case 8:
        if (getKeyStatus('D'))
        {
            return 0x41;
        }
        break;

    case 20:
        return 0x40;

    case 24:
        pdl->Bit = 1;
        return 0x40;

    default:
        return 0x40;
    }

    return 0x40;
} 
