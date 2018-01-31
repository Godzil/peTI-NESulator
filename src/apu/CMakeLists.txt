#
# peTI-NESulator CMake
#
# Created by Manoel TRAPIER.
# Copyright (c) 2003-2018 986-Studio. All rights reserved.
#
# $LastChangedDate$
# $Author$
# $HeadURL$
# $Revision$

add_library(apu SndUnixT.c Sound.c)
#add_library(apu SndAlleg.c Sound.c)
target_link_libraries(apu portaudio_static)
