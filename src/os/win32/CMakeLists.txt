#
# peTI-NESulator CMake
#
# Created by Manoël Trapier.
# Copyright (c) 2002-2019 986-Studio.
#
add_library(oslib loadfile.c graphics.c sound.c io.c)

target_link_libraries(oslib glfw ${OPENGL_glu_LIBRARY})