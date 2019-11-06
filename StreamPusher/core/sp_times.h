#pragma once
#ifndef _SP_TIMES_H_INCLUDED_
#define _SP_TIMES_H_INCLUDED_

#include "sp_core.h"


typedef struct {
	time_t      sec;
	sp_uint_t  msec;
	sp_int_t   gmtoff;
} sp_time_t;


#endif // !_SP_TIMES_H_INCLUDED_
