#pragma once
#ifndef _SP_PROCESS_H_INCLUDED_
#define _SP_PROCESS_H_INCLUDED_

#include "sp_core.h"

typedef DWORD               sp_pid_t;
#define SP_INVALID_PID     0

#define sp_getpid          GetCurrentProcessId
#define sp_getppid()       0
#define sp_log_pid         sp_pid

extern sp_pid_t            sp_pid;
extern sp_pid_t            sp_parent;

#endif // !_SP_PROCESS_H_INCLUDED_
