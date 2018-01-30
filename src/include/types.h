/*
 *  Base type definitions - The TI-NESulator Project
 *  types.h - Taken from the Quick6502 project
 *
 *  Created by Manoel Trapier on 18/09/06.
 *  Copyright (c) 2003-2018 986-Studio. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#ifndef BYTE_TYPE_DEFINED
#define BYTE_TYPE_DEFINED
typedef uint8_t byte;
#endif

typedef uint8_t bool;

#define true  (0)
#define false (!true)

#endif
