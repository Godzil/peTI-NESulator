/*
 * Copyright (c) 2002-2019 986-Studio.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

//#include <sys/mman.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fcntl.h>


/* Map a file in memory */
void *LoadFilePtr(char * filename)
{
	int fd;
	void *RetPtr;
	struct stat FileStat;

	fd = open(filename, O_RDONLY);

	fstat(fd, &FileStat);

	RetPtr = NULL;
//	RetPtr = mmap(NULL, FileStat.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	
	close(fd);

	return RetPtr;
}
