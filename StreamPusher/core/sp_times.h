#pragma once
#ifndef _SP_TIMES_H_INCLUDED_
#define _SP_TIMES_H_INCLUDED_

#include "sp_string.h"

typedef struct {
	time_t      sec;
	sp_uint_t  msec;
	sp_int_t   gmtoff;
} sp_time_t;

void sp_time_init(void);
void sp_time_update(void);
void sp_gmtime(time_t t, sp_tm_t *tp);


#define sp_next_time_n      "mktime()"


extern volatile sp_time_t  *sp_cached_time;

#define sp_time()           sp_cached_time->sec
#define sp_timeofday()      (sp_time_t *) sp_cached_time

extern volatile sp_str_t    sp_cached_err_log_time;
extern volatile sp_str_t    sp_cached_http_time;
extern volatile sp_str_t    sp_cached_http_log_time;
extern volatile sp_str_t    sp_cached_http_log_iso8601;
extern volatile sp_str_t    sp_cached_syslog_time;

/*
 * milliseconds elapsed since epoch and truncated to sp_msec_t,
 * used in event timers
 */
extern volatile sp_msec_t  sp_current_msec;

#endif // !_SP_TIMES_H_INCLUDED_
