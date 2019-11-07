#pragma once
#include "win_time.h"


void
sp_gettimeofday(struct timeval *tp)
{
	uint64_t  intervals;
	FILETIME  ft;

	GetSystemTimeAsFileTime(&ft);

	intervals = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
	intervals -= 116444736000000000;

	tp->tv_sec = (long)(intervals / 10000000);
	tp->tv_usec = (long)((intervals % 10000000) / 10);
}


void
sp_libc_localtime(time_t s, struct tm *tm)
{
	struct tm  *t;

	t = localtime(&s);
	*tm = *t;
}


void
sp_libc_gmtime(time_t s, struct tm *tm)
{
	struct tm  *t;

	t = gmtime(&s);
	*tm = *t;
}


sp_int_t
sp_gettimezone(void)
{
	u_long                 n;
	TIME_ZONE_INFORMATION  tz;

	n = GetTimeZoneInformation(&tz);

	switch (n) {

	case TIME_ZONE_ID_UNKNOWN:
		return -tz.Bias;

	case TIME_ZONE_ID_STANDARD:
		return -(tz.Bias + tz.StandardBias);

	case TIME_ZONE_ID_DAYLIGHT:
		return -(tz.Bias + tz.DaylightBias);

	default: /* TIME_ZONE_ID_INVALID */
		return 0;
	}
}
