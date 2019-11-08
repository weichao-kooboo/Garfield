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


sp_log_t *
sp_log_init(u_char *prefix)
{
	u_char  *p, *name;
	size_t   nlen, plen;

	sp_log.file = &sp_log_file;
	sp_log.log_level = SP_LOG_NOTICE;

	name = (u_char *)SP_ERROR_LOG_PATH;

	/*
	 * we use sp_strlen() here since BCC warns about
	 * condition is always false and unreachable code
	 */

	nlen = sp_strlen(name);

	if (nlen == 0) {
		sp_log_file.fd = sp_stderr;
		return &sp_log;
	}

	p = NULL;

#if (SP_WIN32)
	if (name[1] != ':') {
#else
	if (name[0] != '/') {
#endif

		if (prefix) {
			plen = sp_strlen(prefix);

		}
		else {
#ifdef SP_PREFIX
			prefix = (u_char *)SP_PREFIX;
			plen = sp_strlen(prefix);
#else
			plen = 0;
#endif
		}

		if (plen) {
			name = malloc(plen + nlen + 2);
			if (name == NULL) {
				return NULL;
			}

			p = sp_cpymem(name, prefix, plen);

			if (!sp_path_separator(*(p - 1))) {
				*p++ = '/';
			}

			sp_cpystrn(p, (u_char *)SP_ERROR_LOG_PATH, nlen + 1);

			p = name;
		}
	}

	sp_log_file.fd = sp_open_file(name, SP_FILE_APPEND,
		SP_FILE_CREATE_OR_OPEN,
		SP_FILE_DEFAULT_ACCESS);

	if (sp_log_file.fd == SP_INVALID_FILE) {
		sp_log_stderr(sp_errno,
			"[alert] could not open error log file: "
			sp_open_file_n " \"%s\" failed", name);
#if (SP_WIN32)
		sp_event_log(sp_errno,
			"could not open error log file: "
			sp_open_file_n " \"%s\" failed", name);
#endif

		sp_log_file.fd = sp_stderr;
	}

	if (p) {
		sp_free(p);
	}

	return &sp_log;
}