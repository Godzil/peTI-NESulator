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

##########################
# Configurations variables
##########################
set(PPU_ISPAL OFF CACHE BOOL "When switch to ON the PPU is in PAL mode, else it will act as a NTSC one.")
set(Q6502_DEBUGGER OFF CACHE BOOL "Activate the Quick6502 debugger?")
set(USE_SOUND OFF CACHE BOOL "Activate the sound?")
set(DETECT_BUS_CONFLICT OFF CACHE BOOL "Activate the bus conflit detector? (Could slow down the emulator a lot.)")

set(USE_EFENCE OFF CACHE BOOL "Use electricfence memory debugger?")
set(USE_PROFILING OFF CACHE BOOL "Use profiling tools? (Will slow down a lot.)")

set(USE_ALLEGRO ON CACHE BOOL "Use Allegro backend")

##########################
# Link & Compile flags
##########################

set (CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-parameter -Werror ${PLATFORM_FLAGS}")
set (CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter -Werror ${PLATFORM_FLAGS}")

add_definitions (-DNO_DECIMAL)

if (PPU_ISPAL)
   add_definitions (-DISPAL)
else (PPU_ISPAL)
   add_definitions (-DISNTSC)
endif (PPU_ISPAL)

if (Q6502_DEBUGGER)
   add_definitions (-DDEBUG)
endif (Q6502_DEBUGGER)

if (USE_SOUND)
   add_definitions (-DUSE_SOUND)
endif (USE_SOUND)

if (DETECT_BUS_CONFLICT)
   add_definitions (-DDETECT_BUS_CONFLICT)
endif (DETECT_BUS_CONFLICT)

if (USE_PROFILING)
   if (CMAKE_BUILD_TYPE MATCHES Rel)
      SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Debug info is forced" FORCE)
   else(CMAKE_BUILD_TYPE MATCHES Rel)
      SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Debug info is forced" FORCE)
   endif(CMAKE_BUILD_TYPE MATCHES Rel)

   set(CMAKE_C_FLAGS -pg)
endif (USE_PROFILING)

include_directories(include)

add_subdirectory(apu)
add_subdirectory(corecpu)
add_subdirectory(mappersmanager)
add_subdirectory(memorymanager)
add_subdirectory(pluginsmanager)
add_subdirectory(ppu)


if (TARGET_TI68k)
   add_subdirectory(os/ti68k)
elseif (WIN32)
   add_subdirectory(os/win32)
else (TARGET_TI68k)
   add_subdirectory(os/unix)
endif (TARGET_TI68k)

find_library(PTHREADLIB pthread)

add_executable(tines main.c paddle.c NESCarts.c)
target_compile_options(tines PRIVATE -pthread)
target_link_libraries(tines apu corecpu mappermanager memorymanager pluginsmanager ppu oslib) #${PTHREADLIB}