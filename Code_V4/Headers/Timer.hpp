#ifndef GUARD_TIMER_HPP
#define GUARD_TIMER_HPP

/* Used for timing the code. */

#include <sys/time.h>
#include <ctime>


/* Remove if already defined */
typedef long long int64; typedef unsigned long long uint64;

/* Returns the amount of milliseconds elapsed since the UNIX epoch. */

uint64 GetTimeMs64()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    uint64 ret = tv.tv_usec;

    /* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
    ret /= 1000;

    /* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
    ret += (tv.tv_sec * 1000);

    return ret;
}

#endif
