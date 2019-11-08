#pragma once
#ifndef _SP_LOG_H_INCLUDED_
#define _SP_LOG_H_INCLUDED_

#include "win_files.h"


#define SP_LOG_STDERR            0
#define SP_LOG_EMERG             1
#define SP_LOG_ALERT             2
#define SP_LOG_CRIT              3
#define SP_LOG_ERR               4
#define SP_LOG_WARN              5
#define SP_LOG_NOTICE            6
#define SP_LOG_INFO              7
#define SP_LOG_DEBUG             8

#define SP_LOG_DEBUG_CORE        0x010
#define SP_LOG_DEBUG_ALLOC       0x020
#define SP_LOG_DEBUG_MUTEX       0x040
#define SP_LOG_DEBUG_EVENT       0x080
#define SP_LOG_DEBUG_HTTP        0x100
#define SP_LOG_DEBUG_MAIL        0x200
#define SP_LOG_DEBUG_STREAM      0x400

#define SP_LOG_DEBUG_FIRST       SP_LOG_DEBUG_CORE
#define SP_LOG_DEBUG_LAST        SP_LOG_DEBUG_STREAM
#define SP_LOG_DEBUG_CONNECTION  0x80000000
#define SP_LOG_DEBUG_ALL         0x7ffffff0

typedef struct{
	
}sp_log_t;

#define SP_MAX_ERROR_STR   2048

//将系统错误输出到输出到控制台
void sp_log_stderr(sp_err_t err, const char *fmt, ...);
//返回err代表的系统错误,添加到buf后面
u_char *sp_log_errno(u_char *buf, u_char *last, sp_err_t err);

sp_log_t *sp_log_init(u_char *prefix);

#endif // !_SP_LOG_H_INCLUDED_
