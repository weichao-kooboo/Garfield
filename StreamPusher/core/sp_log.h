#pragma once
#ifndef _SP_LOG_H_INCLUDED_
#define _SP_LOG_H_INCLUDED_

#include "win_files.h"

#define SP_MAX_ERROR_STR   2048

//将系统错误输出到输出到控制台
void sp_log_stderr(sp_err_t err, const char *fmt, ...);
//返回err代表的系统错误,添加到buf后面
u_char *sp_log_errno(u_char *buf, u_char *last, sp_err_t err);

//sp_log_t *sp_log_init(u_char *prefix);

#endif // !_SP_LOG_H_INCLUDED_
