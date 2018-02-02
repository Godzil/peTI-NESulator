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

add_library(oslib loadfile.c graphics.c sound.c io.c)

target_link_libraries(oslib glfw ${OPENGL_glu_LIBRARY})