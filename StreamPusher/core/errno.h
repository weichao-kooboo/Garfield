#pragma once
#ifndef _SP_ERRNO_H_INCLUDED_
#define _SP_ERRNO_H_INCLUDED_

#include "sp_core.h"


typedef int               sp_err_t;

#define SP_EPERM         EPERM
#define SP_ENOENT        ENOENT
#define SP_ENOPATH       ENOENT
#define SP_ESRCH         ESRCH
#define SP_EINTR         EINTR
#define SP_ECHILD        ECHILD
#define SP_ENOMEM        ENOMEM
#define SP_EACCES        EACCES
#define SP_EBUSY         EBUSY
#define SP_EEXIST        EEXIST
#define SP_EEXIST_FILE   EEXIST
#define SP_EXDEV         EXDEV
#define SP_ENOTDIR       ENOTDIR
#define SP_EISDIR        EISDIR
#define SP_EINVAL        EINVAL
#define SP_ENFILE        ENFILE
#define SP_EMFILE        EMFILE
#define SP_ENOSPC        ENOSPC
#define SP_EPIPE         EPIPE
#define SP_EINPROGRESS   EINPROGRESS
#define SP_ENOPROTOOPT   ENOPROTOOPT
#define SP_EOPNOTSUPP    EOPNOTSUPP
#define SP_EADDRINUSE    EADDRINUSE
#define SP_ECONNABORTED  ECONNABORTED
#define SP_ECONNRESET    ECONNRESET
#define SP_ENOTCONN      ENOTCONN
#define SP_ETIMEDOUT     ETIMEDOUT
#define SP_ECONNREFUSED  ECONNREFUSED
#define SP_ENAMETOOLONG  ENAMETOOLONG
#define SP_ENETDOWN      ENETDOWN
#define SP_ENETUNREACH   ENETUNREACH
#define SP_EHOSTDOWN     EHOSTDOWN
#define SP_EHOSTUNREACH  EHOSTUNREACH
#define SP_ENOSYS        ENOSYS
#define SP_ECANCELED     ECANCELED
#define SP_EILSEQ        EILSEQ
#define SP_ENOMOREFILES  0
#define SP_ELOOP         ELOOP
#define SP_EBADF         EBADF

#if (SP_HAVE_OPENAT)
#define SP_EMLINK        EMLINK
#endif

#if (__hpux__)
#define SP_EAGAIN        EWOULDBLOCK
#else
#define SP_EAGAIN        EAGAIN
#endif


#define sp_errno                  errno
#define sp_socket_errno           errno
#define sp_set_errno(err)         errno = err
#define sp_set_socket_errno(err)  errno = err


u_char *sp_strerror(sp_err_t err, u_char *errstr, size_t size);
sp_int_t  sp_strerror_init(void);
#endif // !_SP_ERRNO_H_INCLUDED_