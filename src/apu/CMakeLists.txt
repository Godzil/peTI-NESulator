#
# peTI-NESulator CMake
#
# Created by Manoël Trapier.
# Copyright (c) 2002-2019 986-Studio.
#
add_library(apu apu.c apu.c)
target_link_libraries(apu portaudio_static)
