#pragma once
#ifndef _SP_WIN_FILES_H_INCLUDED_
#define _SP_WIN_FILES_H_INCLUDED_

#include "sp_core.h"

typedef HANDLE                      sp_fd_t;
typedef BY_HANDLE_FILE_INFORMATION  sp_file_info_t;
typedef uint64_t                    sp_file_uniq_t;

#define sp_linefeed(p)             *p++ = CR; *p++ = LF;
#define SP_LINEFEED_SIZE           2
#define SP_LINEFEED                CRLF

//½«buffÐ´Èëµ½fd
ssize_t sp_write_console(sp_fd_t fd, void *buf, size_t size);

#define sp_stdout               GetStdHandle(STD_OUTPUT_HANDLE)
#define sp_stderr               GetStdHandle(STD_ERROR_HANDLE)
#define sp_set_stderr(fd)       SetStdHandle(STD_ERROR_HANDLE, fd)
#define sp_set_stderr_n         "SetStdHandle(STD_ERROR_HANDLE)"

#endif // !_SP_WIN_FILES_H_INCLUDED_
