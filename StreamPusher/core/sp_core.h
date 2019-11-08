#pragma once
#ifndef _SP_CORE_H_INCLUDED_
#define _SP_CORE_H_INCLUDED_

#include <Windows.h>
#include "local_config.h"
#include "win_config.h"

#define SP_MAX_UINT32_VALUE  (uint32_t) 0xffffffff
#define SP_MAX_INT32_VALUE   (uint32_t) 0x7fffffff

#define LF     (u_char) '\n'
#define CR     (u_char) '\r'
#define CRLF   "\r\n"

#define sp_abs(value)       (((value) >= 0) ? (value) : - (value))
#define sp_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))
#define sp_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))

#define  SP_OK          0
#define  SP_ERROR      -1
#define  SP_AGAIN      -2
#define  SP_BUSY       -3
#define  SP_DONE       -4
#define  SP_DECLINED   -5
#define  SP_ABORT      -6

typedef intptr_t        sp_int_t;
typedef uintptr_t       sp_uint_t;
typedef intptr_t        sp_flag_t;

#define SP_INT32_LEN   (sizeof("-2147483648") - 1)
#define SP_INT64_LEN   (sizeof("-9223372036854775808") - 1)

#if (SP_PTR_SIZE == 4)
#define SP_INT_T_LEN   SP_INT32_LEN
#define SP_MAX_INT_T_VALUE  2147483647

#else
#define SP_INT_T_LEN   SP_INT64_LEN
#define SP_MAX_INT_T_VALUE  9223372036854775807
#endif

#ifndef sp_inline
#define sp_inline      inline
#endif


#ifndef SP_SYS_NERR
#define SP_SYS_NERR  135
#endif

#endif // !_SP_CORE_H_INCLUDED_