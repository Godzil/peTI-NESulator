/**
 *  Log Facility - The Quick6502 Project
 *  include/log.h
 *
 *  Created by Manoel Trapier on 19/05/10
 *  Copyright (c) 2003-2016 986-Studio. All rights reserved.
 *
 *  $LastChangedDate:$
 *  $Author:$
 *  $HeadURL:$
 *  $Revision:$
 *
 */

#ifndef _LOG_H
#define	_LOG_H

enum
{
    LOG_ALWAYS = -1,
    LOG_PANIC = 0,
    LOG_ERROR,
    LOG_WARNING,
    LOG_NORMAL,
    LOG_VERBOSE,
    LOG_DEBUG,
};

#define TIME_STAMP_LOG

#define MAX_DEBUG_LEVEL LOG_PANIC
#define log(_level, _user, _fmt, ...) if ((_level <= MAX_DEBUG_LEVEL) || (_level <= LOG_PANIC)) do { log_real(_level, _user, _fmt, ##__VA_ARGS__); } while(0)

void log_real(int level, char *user, char *fmt, ...);

#define LOG(_level, _str, ...) if ((_level <= MAX_DEBUG_LEVEL) || (_level <= LOG_PANIC)) do { puts(_str); } while(0)
#define LOGCODE(_level, _user, _code) log(_level, _user, ""); \
    if ((_level <= MAX_DEBUG_LEVEL) || (_level <= LOG_PANIC)) \
       do { _code; printf("\n"); } while(0)

#endif	/* _LOG_H */

