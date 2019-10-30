#pragma once
#ifndef _SP_WIN_ERRNO_H_INCLUDED_
#define _SP_WIN_ERRNO_H_INCLUDED_

#include "sp_core.h"


typedef DWORD                      sp_err_t;

#define sp_errno                  GetLastError()
#define sp_set_errno(err)         SetLastError(err)
#define sp_socket_errno           WSAGetLastError()
#define sp_set_socket_errno(err)  WSASetLastError(err)

#define SP_EPERM                  ERROR_ACCESS_DENIED
#define SP_ENOENT                 ERROR_FILE_NOT_FOUND
#define SP_ENOPATH                ERROR_PATH_NOT_FOUND
#define SP_ENOMEM                 ERROR_NOT_ENOUGH_MEMORY
#define SP_EACCES                 ERROR_ACCESS_DENIED
/*
 * there are two EEXIST error codes:
 * ERROR_FILE_EXISTS used by CreateFile(CREATE_NEW),
 * and ERROR_ALREADY_EXISTS used by CreateDirectory();
 * MoveFile() uses both
 */
#define SP_EEXIST                 ERROR_ALREADY_EXISTS
#define SP_EEXIST_FILE            ERROR_FILE_EXISTS
#define SP_EXDEV                  ERROR_NOT_SAME_DEVICE
#define SP_ENOTDIR                ERROR_PATH_NOT_FOUND
#define SP_EISDIR                 ERROR_CANNOT_MAKE
#define SP_ENOSPC                 ERROR_DISK_FULL
#define SP_EPIPE                  EPIPE
#define SP_EAGAIN                 WSAEWOULDBLOCK
#define SP_EINPROGRESS            WSAEINPROGRESS
#define SP_ENOPROTOOPT            WSAENOPROTOOPT
#define SP_EOPNOTSUPP             WSAEOPNOTSUPP
#define SP_EADDRINUSE             WSAEADDRINUSE
#define SP_ECONNABORTED           WSAECONNABORTED
#define SP_ECONNRESET             WSAECONNRESET
#define SP_ENOTCONN               WSAENOTCONN
#define SP_ETIMEDOUT              WSAETIMEDOUT
#define SP_ECONNREFUSED           WSAECONNREFUSED
#define SP_ENAMETOOLONG           ERROR_BAD_PATHNAME
#define SP_ENETDOWN               WSAENETDOWN
#define SP_ENETUNREACH            WSAENETUNREACH
#define SP_EHOSTDOWN              WSAEHOSTDOWN
#define SP_EHOSTUNREACH           WSAEHOSTUNREACH
#define SP_ENOMOREFILES           ERROR_NO_MORE_FILES
#define SP_EILSEQ                 ERROR_NO_UNICODE_TRANSLATION
#define SP_ELOOP                  0
#define SP_EBADF                  WSAEBADF

#define SP_EALREADY               WSAEALREADY
#define SP_EINVAL                 WSAEINVAL
#define SP_EMFILE                 WSAEMFILE
#define SP_ENFILE                 WSAEMFILE


u_char *sp_strerror(sp_err_t err, u_char *errstr, size_t size);
sp_int_t  sp_strerror_init(void);
#endif // !_SP_ERRNO_H_INCLUDED_