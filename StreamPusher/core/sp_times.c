#pragma once
#include "sp_times.h"

static sp_msec_t sp_monotonic_time(time_t sec, sp_uint_t msec);

#define SP_TIME_SLOTS   64

static sp_uint_t slot;
static sp_atomic_t sp_time_lock;

volatile sp_msec_t      sp_current_msec;
volatile sp_time_t     *sp_cached_time;
volatile sp_str_t       sp_cached_err_log_time;
volatile sp_str_t       sp_cached_http_time;
volatile sp_str_t       sp_cached_http_log_time;
volatile sp_str_t       sp_cached_http_log_iso8601;
volatile sp_str_t       sp_cached_syslog_time;


static char  *week[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char  *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
						   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

#if !(SP_WIN32)

/*
 * localtime() and localtime_r() are not Async-Signal-Safe functions, therefore,
 * they must not be called by a signal handler, so we use the cached
 * GMT offset value. Fortunately the value is changed only two times a year.
 */

static sp_int_t         cached_gmtoff;
#endif


static sp_time_t        cached_time[SP_TIME_SLOTS];
static u_char            cached_err_log_time[SP_TIME_SLOTS]
[sizeof("1970/09/28 12:00:00")];
static u_char            cached_http_time[SP_TIME_SLOTS]
[sizeof("Mon, 28 Sep 1970 06:00:00 GMT")];
static u_char            cached_http_log_time[SP_TIME_SLOTS]
[sizeof("28/Sep/1970:12:00:00 +0600")];
static u_char            cached_http_log_iso8601[SP_TIME_SLOTS]
[sizeof("1970-09-28T12:00:00+06:00")];
static u_char            cached_syslog_time[SP_TIME_SLOTS]
[sizeof("Sep 28 12:00:00")];


void sp_time_init(void) {
	sp_cached_err_log_time.len = sizeof("1970/09/28 12:00:00") - 1;
	sp_cached_http_time.len = sizeof("Mon, 28 Sep 1970 06:00:00 GMT") - 1;
	sp_cached_http_log_time.len = sizeof("28/Sep/1970:12:00:00 +0600") - 1;
	sp_cached_http_log_iso8601.len = sizeof("1970-09-28T12:00:00+06:00") - 1;
	sp_cached_syslog_time.len = sizeof("Sep 28 12:00:00") - 1;

	sp_cached_time = &cached_time[0];

	sp_time_update();
}

void sp_time_update(void) {
	u_char          *p0, *p1, *p2, *p3, *p4;
	sp_tm_t         tm, gmt;
	time_t           sec;
	sp_uint_t       msec;
	sp_time_t      *tp;
	struct timeval   tv;

	if (!sp_trylock(&sp_time_lock)) {
		return;
	}

	sp_gettimeofday(&tv);

	sec = tv.tv_sec;
	msec = tv.tv_usec / 1000;

	sp_current_msec = sp_monotonic_time(sec, msec);

	tp = &cached_time[slot];

	if (tp->sec == sec) {
		tp->msec = msec;
		sp_unlock(&sp_time_lock);
		return;
	}

	if (slot == SP_TIME_SLOTS - 1) {
		slot = 0;
	}
	else {
		slot++;
	}

	tp = &cached_time[slot];

	tp->sec = sec;
	tp->msec = msec;

	sp_gmtime(sec, &gmt);


	p0 = &cached_http_time[slot][0];

	(void)sp_sprintf(p0, "%s, %02d %s %4d %02d:%02d:%02d GMT",
		week[gmt.sp_tm_wday], gmt.sp_tm_mday,
		months[gmt.sp_tm_mon - 1], gmt.sp_tm_year,
		gmt.sp_tm_hour, gmt.sp_tm_min, gmt.sp_tm_sec);

#if (SP_HAVE_GETTIMEZONE)

	tp->gmtoff = sp_gettimezone();
	sp_gmtime(sec + tp->gmtoff * 60, &tm);

#elif (SP_HAVE_GMTOFF)

	sp_localtime(sec, &tm);
	cached_gmtoff = (sp_int_t)(tm.sp_tm_gmtoff / 60);
	tp->gmtoff = cached_gmtoff;

#else

	sp_localtime(sec, &tm);
	cached_gmtoff = sp_timezone(tm.sp_tm_isdst);
	tp->gmtoff = cached_gmtoff;

#endif


	p1 = &cached_err_log_time[slot][0];

	(void)sp_sprintf(p1, "%4d/%02d/%02d %02d:%02d:%02d",
		tm.sp_tm_year, tm.sp_tm_mon,
		tm.sp_tm_mday, tm.sp_tm_hour,
		tm.sp_tm_min, tm.sp_tm_sec);


	p2 = &cached_http_log_time[slot][0];

	(void)sp_sprintf(p2, "%02d/%s/%d:%02d:%02d:%02d %c%02i%02i",
		tm.sp_tm_mday, months[tm.sp_tm_mon - 1],
		tm.sp_tm_year, tm.sp_tm_hour,
		tm.sp_tm_min, tm.sp_tm_sec,
		tp->gmtoff < 0 ? '-' : '+',
		sp_abs(tp->gmtoff / 60), sp_abs(tp->gmtoff % 60));

	p3 = &cached_http_log_iso8601[slot][0];

	(void)sp_sprintf(p3, "%4d-%02d-%02dT%02d:%02d:%02d%c%02i:%02i",
		tm.sp_tm_year, tm.sp_tm_mon,
		tm.sp_tm_mday, tm.sp_tm_hour,
		tm.sp_tm_min, tm.sp_tm_sec,
		tp->gmtoff < 0 ? '-' : '+',
		sp_abs(tp->gmtoff / 60), sp_abs(tp->gmtoff % 60));

	p4 = &cached_syslog_time[slot][0];

	(void)sp_sprintf(p4, "%s %2d %02d:%02d:%02d",
		months[tm.sp_tm_mon - 1], tm.sp_tm_mday,
		tm.sp_tm_hour, tm.sp_tm_min, tm.sp_tm_sec);

	sp_memory_barrier();

	sp_cached_time = tp;
	sp_cached_http_time.data = p0;
	sp_cached_err_log_time.data = p1;
	sp_cached_http_log_time.data = p2;
	sp_cached_http_log_iso8601.data = p3;
	sp_cached_syslog_time.data = p4;

	sp_unlock(&sp_time_lock);
}


void
sp_gmtime(time_t t, sp_tm_t *tp)
{
	sp_int_t   yday;
	sp_uint_t  sec, min, hour, mday, mon, year, wday, days, leap;

	/* the calculation is valid for positive time_t only */

	if (t < 0) {
		t = 0;
	}

	days = t / 86400;
	sec = t % 86400;

	/*
	 * no more than 4 year digits supported,
	 * truncate to December 31, 9999, 23:59:59
	 */

	if (days > 2932896) {
		days = 2932896;
		sec = 86399;
	}

	/* January 1, 1970 was Thursday */

	wday = (4 + days) % 7;

	hour = sec / 3600;
	sec %= 3600;
	min = sec / 60;
	sec %= 60;

	/*
	 * the algorithm based on Gauss' formula,
	 * see src/core/sp_parse_time.c
	 */

	 /* days since March 1, 1 BC */
	days = days - (31 + 28) + 719527;

	/*
	 * The "days" should be adjusted to 1 only, however, some March 1st's go
	 * to previous year, so we adjust them to 2.  This causes also shift of the
	 * last February days to next year, but we catch the case when "yday"
	 * becomes negative.
	 */

	year = (days + 2) * 400 / (365 * 400 + 100 - 4 + 1);

	yday = days - (365 * year + year / 4 - year / 100 + year / 400);

	if (yday < 0) {
		leap = (year % 4 == 0) && (year % 100 || (year % 400 == 0));
		yday = 365 + leap + yday;
		year--;
	}

	/*
	 * The empirical formula that maps "yday" to month.
	 * There are at least 10 variants, some of them are:
	 *     mon = (yday + 31) * 15 / 459
	 *     mon = (yday + 31) * 17 / 520
	 *     mon = (yday + 31) * 20 / 612
	 */

	mon = (yday + 31) * 10 / 306;

	/* the Gauss' formula that evaluates days before the month */

	mday = yday - (367 * mon / 12 - 30) + 1;

	if (yday >= 306) {

		year++;
		mon -= 10;

		/*
		 * there is no "yday" in Win32 SYSTEMTIME
		 *
		 * yday -= 306;
		 */

	}
	else {

		mon += 2;

		/*
		 * there is no "yday" in Win32 SYSTEMTIME
		 *
		 * yday += 31 + 28 + leap;
		 */
	}

	tp->sp_tm_sec = (sp_tm_sec_t)sec;
	tp->sp_tm_min = (sp_tm_min_t)min;
	tp->sp_tm_hour = (sp_tm_hour_t)hour;
	tp->sp_tm_mday = (sp_tm_mday_t)mday;
	tp->sp_tm_mon = (sp_tm_mon_t)mon;
	tp->sp_tm_year = (sp_tm_year_t)year;
	tp->sp_tm_wday = (sp_tm_wday_t)wday;
}

static sp_msec_t
sp_monotonic_time(time_t sec, sp_uint_t msec)
{
#if (SP_HAVE_CLOCK_MONOTONIC)
	struct timespec  ts;

#if defined(CLOCK_MONOTONIC_FAST)
	clock_gettime(CLOCK_MONOTONIC_FAST, &ts);

#elif defined(CLOCK_MONOTONIC_COARSE)
	clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);

#else
	clock_gettime(CLOCK_MONOTONIC, &ts);
#endif

	sec = ts.tv_sec;
	msec = ts.tv_nsec / 1000000;

#endif

	return (sp_msec_t)sec * 1000 + msec;
}