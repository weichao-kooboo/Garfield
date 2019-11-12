#pragma once
#ifndef _SP_WIN_FILES_H_INCLUDED_
#define _SP_WIN_FILES_H_INCLUDED_

#include "win_errno.h"

#define SP_UTF16_BUFLEN  256
#define SP_INVALID_FILE            INVALID_HANDLE_VALUE

typedef HANDLE                      sp_fd_t;
typedef BY_HANDLE_FILE_INFORMATION  sp_file_info_t;
typedef uint64_t                    sp_file_uniq_t;

#define sp_linefeed(p)             *p++ = CR; *p++ = LF;
#define SP_LINEFEED_SIZE           2
#define SP_LINEFEED                CRLF

sp_fd_t sp_open_file(u_char *name, u_long mode, u_long create, u_long access);
#define sp_open_file_n             "CreateFile()"

#define SP_FILE_RDONLY             GENERIC_READ
#define SP_FILE_WRONLY             GENERIC_WRITE
#define SP_FILE_RDWR               GENERIC_READ|GENERIC_WRITE
#define SP_FILE_APPEND             FILE_APPEND_DATA|SYNCHRONIZE
#define SP_FILE_NONBLOCK           0

#define SP_FILE_CREATE_OR_OPEN     OPEN_ALWAYS
#define SP_FILE_OPEN               OPEN_EXISTING
#define SP_FILE_TRUNCATE           CREATE_ALWAYS

#define SP_FILE_DEFAULT_ACCESS     0
#define SP_FILE_OWNER_ACCESS       0

//将buff写入到fd
ssize_t sp_write_console(sp_fd_t fd, void *buf, size_t size);
//将buff写入到文件
ssize_t sp_write_fd(sp_fd_t fd, void *buf, size_t size);
#define sp_write_fd_n              "WriteFile()"

#define sp_stdout               GetStdHandle(STD_OUTPUT_HANDLE)
#define sp_stderr               GetStdHandle(STD_ERROR_HANDLE)
#define sp_set_stderr(fd)       SetStdHandle(STD_ERROR_HANDLE, fd)
#define sp_set_stderr_n         "SetStdHandle(STD_ERROR_HANDLE)"

#define sp_path_separator(c)       ((c) == '/' || (c) == '\\')

#endif // !_SP_WIN_FILES_H_INCLUDED_
