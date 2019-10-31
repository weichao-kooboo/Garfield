#pragma once
#ifndef _SP_LOG_H_INCLUDED_
#define _SP_LOG_H_INCLUDED_
#include "sp_core.h"

#define SP_MAX_ERROR_STR   2048

void sp_log_stderr(sp_err_t err, const char *fmt, ...);
u_char *sp_log_errno(u_char *buf, u_char *last, sp_err_t err);

#endif // !_SP_LOG_H_INCLUDED_
