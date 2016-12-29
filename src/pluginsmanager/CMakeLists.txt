#
# TI-NES CMake
#
# Created by Manoel TRAPIER.
# Copyright (c) 2003-2016 986-Studio. All rights reserved.
#
# $LastChangedDate$
# $Author$
# $HeadURL$
# $Revision$

file(GLOB plugins_list plugins/*.c)
add_library(pluginsmanager manager.c ${plugins_list})