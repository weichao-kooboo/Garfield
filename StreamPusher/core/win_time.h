#pragma once
#ifndef _SP_TIME_H_INCLUDED_
#define	_SP_TIME_H_INCLUDED_

#include "sp_rbtree.h"

/*
* ¿ØÖÆÊ±Çø
*/
typedef sp_rbtree_key_t      sp_msec_t;
typedef sp_rbtree_key_int_t  sp_msec_int_t;

typedef SYSTEMTIME            sp_tm_t;
typedef FILETIME              sp_mtime_t;

#define sp_tm_sec            wSecond
#define sp_tm_min            wMinute
#define sp_tm_hour           wHour
#define sp_tm_mday           wDay
#define sp_tm_mon            wMonth
#define sp_tm_year           wYear
#define sp_tm_wday           wDayOfWeek

#define sp_tm_sec_t          u_short
#define sp_tm_min_t          u_short
#define sp_tm_hour_t         u_short
#define sp_tm_mday_t         u_short
#define sp_tm_mon_t          u_short
#define sp_tm_year_t         u_short
#define sp_tm_wday_t         u_short


#define sp_msleep            Sleep

#define SP_HAVE_GETTIMEZONE  1

#define  sp_timezone_update()

sp_int_t sp_gettimezone(void);
void sp_libc_localtime(time_t s, struct tm *tm);
void sp_libc_gmtime(time_t s, struct tm *tm);
void sp_gettimeofday(struct timeval *tp);

#endif // !_SP_TIME_H_INCLUDED_
