#pragma once
#ifndef _SP_STRING_H_INCLUDED_
#define _SP_STRING_H_INCLUDED_
#include "win_atomic.h"
#include "win_time.h"

typedef struct {
	size_t      len;
	u_char     *data;
} sp_str_t;

typedef struct {
	unsigned    len : 28;

	unsigned    valid : 1;
	unsigned    no_cacheable : 1;
	unsigned    not_found : 1;
	unsigned    escape : 1;

	u_char     *data;
} sp_variable_value_t;

#define sp_string(str)     { sizeof(str) - 1, (u_char *) str }
#define sp_memcpy(dst, src, n)   (void) memcpy(dst, src, n)
#define sp_cpymem(dst, src, n)   (((u_char *) memcpy(dst, src, n)) + (n))


#define sp_strlen(s)       strlen((const char *) s)

//转码
uint32_t sp_utf8_decode(u_char **p, size_t n);
//字符串格式化
u_char *sp_vslprintf(u_char *buf, u_char *last, const char *fmt, va_list args);
u_char * sp_cdecl sp_slprintf(u_char *buf, u_char *last, const char *fmt,
	...);
u_char * sp_cdecl sp_snprintf(u_char *buf, size_t max, const char *fmt, ...);
u_char * sp_cdecl sp_sprintf(u_char *buf, const char *fmt, ...);

#endif // !_SP_STRING_H_INCLUDED_