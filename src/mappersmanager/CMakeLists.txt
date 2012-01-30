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

file(GLOB mappers_list mappers/*.c)
add_library(mappermanager utils.c manager.c ${mappers_list} )