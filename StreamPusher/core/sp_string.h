#pragma once
#ifndef _SP_STRING_H_INCLUDED_
#define _SP_STRING_H_INCLUDED_

typedef struct {
	size_t      len;
	u_char     *data;
} sp_str_t;

#define sp_string(str)     { sizeof(str) - 1, (u_char *) str }
#define sp_cpymem(dst, src, n)   (((u_char *) memcpy(dst, src, n)) + (n))

#endif // !_SP_STRING_H_INCLUDED_