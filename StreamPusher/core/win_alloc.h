#pragma once
#ifndef _SP_WIN_ALLOC_H_INCLUDED_
#define	_SP_WIN_ALLOC_H_INCLUDED_
#include "sp_core.h"

#define sp_free          free
#define sp_memalign(alignment, size, log)  sp_alloc(size, log)

#endif // !_SP_WIN_ALLOC_H_INCLUDED_
