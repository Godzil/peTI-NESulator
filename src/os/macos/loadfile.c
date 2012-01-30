/*
 *  File functions - The TI-NESulator Project
 *  os/macos/load.c
 *
 *  Copyright (c) 2003-2008 986Corp. All rights reserved.
 *
 *  $LastChangedDate$
 *  $Author$
 *  $HeadURL$
 *  $Revision$
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <sys/mman.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fcntl.h>

#include <os_dependent.h>

/* Map a file in memory */
void *LoadFilePtr(char * filename)
{
	int fd;
	void *RetPtr = NULL;
	struct stat FileStat;

	fd = open(filename, O_RDONLY);

	fstat(fd, &FileStat);

	RetPtr = mmap(NULL, FileStat.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	
	close(fd);

	return RetPtr;
}
