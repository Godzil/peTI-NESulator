#
# TI-NES CMake
#
# Created by Manoel TRAPIER.
# Copyright (c) 2003-2008 986Corp. All rights reserved.
#
# $LastChangedDate$
# $Author$
# $HeadURL$
# $Revision$

file(GLOB ppu_debug_list debug/*.c)
add_library(ppu ppu.c ppu.memory.c ${ppu_debug_list})