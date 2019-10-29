#pragma once
#pragma once
#ifndef _SP_CORE_H_INCLUDED_
#define _SP_CORE_H_INCLUDED_

#include <Windows.h>
#include "sp_string.h"

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


#ifndef SP_SYS_NERR
#define SP_SYS_NERR  135
#endif

#endif // !_SP_CORE_H_INCLUDED_