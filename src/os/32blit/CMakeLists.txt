#
# peTI-NESulator CMake
#
# Created by Manoël Trapier.
# Copyright (c) 2002-2019 986-Studio.
#
set(USE_GLFW OFF BOOL CACHE INTERNAL "USE_GLFW")
set(USE_PORTAUDIO OFF BOOL CACHE INTERNAL "USE_PORTAUDIO")

set(PATH_32BLIT "../32blit" CACHE PATH "Path to 32Blit SDK")
set(CMAKE_TOOLCHAIN_FILE "../../"${PATH_32BLIT}/32blit.toolchain)


include("../../"${PATH_32BLIT}/32blit.cmake)

add_library(oslib loadfile.c graphics.c sound.c io.c text.c)
target_link_libraries(oslib)
