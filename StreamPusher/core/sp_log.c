#pragma once
#include "sp_log.h"

void sp_log_stderr(sp_err_t err, const char * fmt, ...)
{
	u_char   *p, *last;
	va_list   args;
	u_char    errstr[SP_MAX_ERROR_STR];

	last = errstr + SP_MAX_ERROR_STR;

	p = sp_cpymem(errstr, "streamPusher: ", 14);

	va_start(args, fmt);
	p = sp_vslprintf(p, last, fmt, args);
	va_end(args);

	if (err) {
		p = sp_log_errno(p, last, err);
	}

	if (p > last - SP_LINEFEED_SIZE) {
		p = last - SP_LINEFEED_SIZE;
	}

	sp_linefeed(p);

	(void)sp_write_console(sp_stderr, errstr, p - errstr);
}

u_char *
sp_log_errno(u_char *buf, u_char *last, sp_err_t err)
{
	if (buf > last - 50) {

		/* leave a space for an error code */

		buf = last - 50;
		*buf++ = '.';
		*buf++ = '.';
		*buf++ = '.';
	}

#if (SP_WIN32)
	buf = sp_slprintf(buf, last, ((unsigned)err < 0x80000000)
		? " (%d: " : " (%Xd: ", err);
#else
	buf = sp_slprintf(buf, last, " (%d: ", err);
#endif

	buf = sp_strerror(err, buf, last - buf);

	if (buf < last) {
		*buf++ = ')';
	}

	return buf;
}