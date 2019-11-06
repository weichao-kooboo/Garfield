#pragma once
#include "sp_times.h"

#define SP_TIME_SLOTS   64

static sp_uint_t slot;
static sp_atomic_t sp_time_lock;



static char  *week[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char  *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
						   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
