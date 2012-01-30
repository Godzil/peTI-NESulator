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

include_directories($(TINES_SOURCE_DIR)/include)

##########################
# Configurations variables
##########################
set(PPU_ISPAL OFF CACHE BOOL "When switch to ON the PPU is in PAL mode, else it will act as a NTSC one.")
set(Q6502_DEBUGGER OFF CACHE BOOL "Activate the Quick6502 debugger?")
set(USE_SOUND OFF CACHE BOOL "Activate the sound?")
set(DETECT_BUS_CONFLICT OFF CACHE BOOL "Activate the bus conflit detector? (Could slow down the emulator a lot.)")

set(USE_EFENCE OFF CACHE BOOL "Use electricfence memory debugger?")
set(USE_PROFILING OFF CACHE BOOL "Use profiling tools? (Will slow down a lot.)")

set(USE_ALLEGRO ON CACHE BOOL "Use Allegro backend" FORCE)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

if (APPLE)
   SET (CMAKE_FIND_FRAMEWORK LAST)
endif (APPLE)

##########################
# Link & Compile flags
##########################

add_definitions (-DNO_DECIMAL -DFAST_RDOP)

SET ( CMAKE_EXE_LINKER_FLAGS "-mmacosx-version-min=10.4")

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

if (USE_EFENCE)
   if (CMAKE_BUILD_TYPE MATCHES Release)
      SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Debug info is forced" FORCE)
   else(CMAKE_BUILD_TYPE)
      SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Debug info is forced" FORCE)
   endif(CMAKE_BUILD_TYPE)
   
endif (USE_EFENCE)

if (USE_PROFILING)
   if (CMAKE_BUILD_TYPE MATCHES Rel)
      SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Debug info is forced" FORCE)
   else(CMAKE_BUILD_TYPE MATCHES Rel)
      SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Debug info is forced" FORCE)
   endif(CMAKE_BUILD_TYPE MATCHES Rel)

   set(CMAKE_C_FLAGS -pg)
endif (USE_PROFILING)

if (APPLE)
   include_directories(BEFORE /usr/include)
endif (APPLE)

#if the CPU is LSB set the define
if (CMAKE_SYSTEM_PROCESSOR MATCHES i386 OR CMAKE_SYSTEM_PROCESSOR MATCHES [aA][rR][mM])
   add_definitions (-DLSB_FIRST)
endif (CMAKE_SYSTEM_PROCESSOR MATCHES i386 OR CMAKE_SYSTEM_PROCESSOR MATCHES [aA][rR][mM])

#Add release mode extra C Flags
set (CMAKE_C_FLAGS_RELEASE "-fomit-frame-pointer -funroll-loops -Wall ${CMAKE_C_FLAGS_RELEASE}")
set (CMAKE_C_FLAGS_RELWITHDEBINFO "-fomit-frame-pointer -funroll-loops -Wall ${CMAKE_C_FLAGS_RELWITHDEBINFO}")

add_subdirectory(apu)
add_subdirectory(corecpu)
add_subdirectory(mappersmanager)
add_subdirectory(memorymanager)
add_subdirectory(pluginsmanager)
add_subdirectory(ppu)


if (TARGET_TI68k)
   add_subdirectory(os/ti68k)
elseif (APPLE AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   add_subdirectory(os/macos)
elseif (UNIX)
   add_subdirectory(os/win32)
else (TARGET_TI68k)
   #So we target UNIX like OS
   add_subdirectory(os/unix)
endif (TARGET_TI68k)


add_library (main main.c paddle.c NESCarts.c)

add_executable(tines main.c)
set(CMAKE_FIND_FRAMEWORK LAST)
find_library(ALLEGROLIB allegro)
find_library(PTHREADLIB pthread)
   
if (USE_EFENCE)
   find_library(EFENCELIB efence)
   target_link_libraries(tines ${EFENCELIB})
endif (USE_EFENCE)

if (USE_ALLEGRO)
   target_link_libraries(tines debug alld-main)
   target_link_libraries(tines optimized alleg-main)
   if (APPLE)
      find_library(COCOALIB Cocoa)
      target_link_libraries(tines ${COCOALIB})
   endif (APPLE)
   
endif (USE_ALLEGRO)

target_link_libraries(tines main apu corecpu mappermanager memorymanager pluginsmanager ppu oslib ${ALLEGROLIB} ${PTHREADLIB})
