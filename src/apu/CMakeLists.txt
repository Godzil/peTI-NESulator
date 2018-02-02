#
# peTI-NESulator CMake
#
# Created by Manoël TRAPIER.
# Copyright (c) 2003-2018 986-Studio. All rights reserved.
#
# $LastChangedDate$
# $Author$
# $HeadURL$
# $Revision$

add_library(apu apu.c apu.c)
target_link_libraries(apu portaudio_static)
