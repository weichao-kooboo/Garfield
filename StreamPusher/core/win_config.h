#pragma once
#ifndef _SP_WIN_CONFIG_H_INCLUDED_
#define _SP_WIN_CONFIG_H_INCLUDED_

#ifdef _WIN64
typedef __int64             ssize_t;
#else
typedef int                 ssize_t;
#endif
typedef __int64             int64_t;
typedef unsigned __int64    uint64_t;


#ifdef _MSC_VER
typedef unsigned __int32    uint32_t;
typedef __int32             int32_t;
typedef unsigned __int16    uint16_t;
#define sp_libc_cdecl      __cdecl

#elif defined __BORLANDC__
typedef unsigned __int32    uint32_t;
typedef __int32             int32_t;
typedef unsigned __int16    uint16_t;
#define sp_libc_cdecl      __cdecl

#else /* __WATCOMC__ */
typedef unsigned int        uint32_t;
typedef int                 int32_t;
typedef unsigned short int  uint16_t;
#define sp_libc_cdecl

#endif

#define sp_inline          __inline
#define sp_cdecl           __cdecl

#define SP_WIN32		   1

#ifndef __MINGW64_VERSION_MAJOR
typedef __int64             sp_off_t;
//#define _OFF_T_DEFINED
#endif

typedef DWORD               sp_pid_t;

#endif // !_SP_WIN_CONFIG_H_INCLUDED_