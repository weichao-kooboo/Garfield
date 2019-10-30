#pragma once
#ifndef _SP_STRING_H_INCLUDED_
#define _SP_STRING_H_INCLUDED_
#include "sp_core.h"

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
#define sp_cpymem(dst, src, n)   (((u_char *) memcpy(dst, src, n)) + (n))

//�ַ�����ʽ��
u_char *sp_vslprintf(u_char *buf, u_char *last, const char *fmt, va_list args);

#endif // !_SP_STRING_H_INCLUDED_