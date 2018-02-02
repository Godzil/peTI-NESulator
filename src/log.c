/**
 *  Log Facility - The Quick6502 Project
 *  log.c
 *
 *  Created by Manoël Trapier on 19/05/10
 *  Copyright 2010 986 Corp. All rights reserved.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <color.h>
#include <string.h>
#include <stdarg.h>
#include <log.h>
#include <sys/time.h>
#include <time.h>

#ifdef TIME_STAMP_LOG

void time_stamp_line(void)
{
    /* Time "0" will be thefirst log line */
    static char firstRun = 1;
    static struct timeval firstTime;
    struct timeval curTime;

    int cMin, cSec;
    long long cMSec;

    /* Get datetime */
    gettimeofday(&curTime, NULL);

    if (firstRun == 1)
    {
        firstRun = 0;
        firstTime.tv_sec = curTime.tv_sec;
        firstTime.tv_usec = curTime.tv_usec;
    }

    cMSec = ((curTime.tv_sec - firstTime.tv_sec) * 1000) + (curTime.tv_usec - firstTime.tv_usec) / 1000;
    cSec = (cMSec / 1000);
    cMSec %= 1000;

    cMin = cSec / 60;

    cSec %= 60;

    /* Put cursor at start of line */
    printf("%c[s", 0x1B);
    printf("%c[7000D", 0x1B);
    printf("%c[1C", 0x1B);
    printf(FWHITE"[" FYELLOW "%03d" FRED "." FBLUE "%02d" FRED "." FGREEN "%03lld" FWHITE "]" CNORMAL, cMin, cSec,
           cMSec);
    printf("%c[u", 0x1B);
}

#endif /* TIME_STAMP_LOG */

void log_real(int level, char *user, char *fmt, ...)
{
    /* The LOG_PANIC must always be displayed */
    if ((level <= MAX_DEBUG_LEVEL) || (level <= LOG_PANIC))
    {
        va_list va;

        switch (level)
        {
        case LOG_PANIC:
            printf(BRED FWHITE);
            break;
        case LOG_ERROR:
            printf(FRED);
            break;
        case LOG_WARNING:
            printf(FYELLOW);
            break;
        default:
        case LOG_NORMAL:
            printf(FGREEN);
            break;
        case LOG_VERBOSE:
            printf(FCYAN);
            break;
        case LOG_DEBUG:
            printf(BBLUE FWHITE);
            break;
        }

#ifdef TIME_STAMP_LOG
        printf("           ");
#endif

        if (user != NULL)
        {
            int i;
            i = strlen(user);
            if (i < 12)
            {
                i = 12 - i;
                for (; i >= 0 ; i--)
                {
                    putchar(' ');
                }
            }
            printf("%s", user);
        }
        else
        {
            switch (level)
            {
            case LOG_PANIC:
                printf("   PANIC");
                break;
            case LOG_ERROR:
                printf("   Error");
                break;
            case LOG_WARNING:
                printf(" Warning");
                break;
            default:
            case LOG_NORMAL:
                printf("    Info");
                break;
            case LOG_VERBOSE:
                printf(" Verbose");
                break;
            case LOG_DEBUG:
                printf("   Debug");
                break;
            }
        }

        printf(CNORMAL ": ");

#ifdef TIME_STAMP_LOG
        time_stamp_line();
#endif /* TIME_STAMP_LOG */

        va_start(va, fmt);
        vprintf(fmt, va);
        va_end(va);

        if (fmt[0] != 0)
        {
            printf("\n");
        }
    }
}
