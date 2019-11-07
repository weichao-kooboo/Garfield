#pragma once
#ifndef _SP_WIN_ATOMIC_H_INCLUDED_
#define	_SP_WIN_ATOMIC_H_INCLUDED_
#include "sp_core.h"

#define SP_HAVE_ATOMIC_OPS   1

typedef int32_t                     sp_atomic_int_t;
typedef uint32_t                    sp_atomic_uint_t;
typedef volatile sp_atomic_uint_t  sp_atomic_t;
#define SP_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)


#if defined( __WATCOMC__ ) || defined( __BORLANDC__ ) || defined(__GNUC__)    \
    || ( _MSC_VER >= 1300 )

/* the new SDK headers */

#define sp_atomic_cmp_set(lock, old, set)                                    \
    ((sp_atomic_uint_t) InterlockedCompareExchange((long *) lock, set, old)  \
                         == old)

#else

/* the old MS VC6.0SP2 SDK headers */

#define sp_atomic_cmp_set(lock, old, set)                                    \
    (InterlockedCompareExchange((void **) lock, (void *) set, (void *) old)   \
     == (void *) old)

#endif


#define sp_trylock(lock)  (*(lock) == 0 && sp_atomic_cmp_set(lock, 0, 1))
#define sp_unlock(lock)    *(lock) = 0

#endif // !_SP_WIN_ATOMIC_H_INCLUDED_
