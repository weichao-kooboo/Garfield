#pragma once
#ifndef _SP_WIN_ATOMIC_H_INCLUDED_
#define	_SP_WIN_ATOMIC_H_INCLUDED_
#include "sp_core.h"

#define SP_HAVE_ATOMIC_OPS   1

typedef int32_t                     sp_atomic_int_t;
typedef uint32_t                    sp_atomic_uint_t;
typedef volatile sp_atomic_uint_t  sp_atomic_t;
#define SP_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)

#endif // !_SP_WIN_ATOMIC_H_INCLUDED_
