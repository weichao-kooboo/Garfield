#pragma once
#include "sp_log.h"

static sp_log_t        sp_log;
static sp_open_file_t  sp_log_file;
sp_uint_t              sp_use_stderr = 1;
sp_pid_t            sp_pid;

static sp_str_t err_levels[] = {
	sp_null_string,
	sp_string("emerg"),
	sp_string("alert"),
	sp_string("crit"),
	sp_string("error"),
	sp_string("warn"),
	sp_string("notice"),
	sp_string("info"),
	sp_string("debug")
};

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

sp_log_t * sp_log_init(u_char * prefix)
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


#if (SP_HAVE_VARIADIC_MACROS)

void
sp_log_error_core(sp_uint_t level, sp_log_t *log, sp_err_t err,
	const char *fmt, ...)

#else

void
sp_log_error_core(sp_uint_t level, sp_log_t *log, sp_err_t err,
	const char *fmt, va_list args)

#endif
{
#if (SP_HAVE_VARIADIC_MACROS)
	va_list      args;
#endif
	u_char      *p, *last, *msg;
	ssize_t      n;
	sp_uint_t   wrote_stderr, debug_connection;
	u_char       errstr[SP_MAX_ERROR_STR];

	last = errstr + SP_MAX_ERROR_STR;

	p = sp_cpymem(errstr, sp_cached_err_log_time.data,
		sp_cached_err_log_time.len);

	p = sp_slprintf(p, last, " [%V] ", &err_levels[level]);

	/* pid#tid */
	// ?暂时不加线程名称
	p = sp_slprintf(p, last, "%P: ",
		sp_log_pid);

	if (log->connection) {
		p = sp_slprintf(p, last, "*%uA ", log->connection);
	}

	msg = p;

#if (SP_HAVE_VARIADIC_MACROS)

	va_start(args, fmt);
	p = sp_vslprintf(p, last, fmt, args);
	va_end(args);

#else

	p = sp_vslprintf(p, last, fmt, args);

#endif

	if (err) {
		p = sp_log_errno(p, last, err);
	}

	if (level != SP_LOG_DEBUG && log->handler) {
		p = log->handler(log, p, last - p);
	}

	if (p > last - SP_LINEFEED_SIZE) {
		p = last - SP_LINEFEED_SIZE;
	}

	sp_linefeed(p);

	wrote_stderr = 0;
	debug_connection = (log->log_level & SP_LOG_DEBUG_CONNECTION) != 0;

	while (log) {

		if (log->log_level < level && !debug_connection) {
			break;
		}

		if (log->writer) {
			log->writer(log, level, errstr, p - errstr);
			goto next;
		}

		if (sp_time() == log->disk_full_time) {

			/*
			 * on FreeBSD writing to a full filesystem with enabled softupdates
			 * may block process for much longer time than writing to non-full
			 * filesystem, so we skip writing to a log for one second
			 */

			goto next;
		}

		n = sp_write_fd(log->file->fd, errstr, p - errstr);

		if (n == -1 && sp_errno == SP_ENOSPC) {
			log->disk_full_time = sp_time();
		}

		if (log->file->fd == sp_stderr) {
			wrote_stderr = 1;
		}

	next:

		log = log->next;
	}

	if (!sp_use_stderr
		|| level > SP_LOG_WARN
		|| wrote_stderr)
	{
		return;
	}

	msg -= (7 + err_levels[level].len + 3);

	(void)sp_sprintf(msg, "nginx: [%V] ", &err_levels[level]);

	(void)sp_write_console(sp_stderr, msg, p - msg);
}


#if !(SP_HAVE_VARIADIC_MACROS)

void sp_cdecl
sp_log_error(sp_uint_t level, sp_log_t *log, sp_err_t err,
	const char *fmt, ...)
{
	va_list  args;

	if (log->log_level >= level) {
		va_start(args, fmt);
		sp_log_error_core(level, log, err, fmt, args);
		va_end(args);
	}
}


void sp_cdecl
sp_log_debug_core(sp_log_t *log, sp_err_t err, const char *fmt, ...)
{
	va_list  args;

	va_start(args, fmt);
	sp_log_error_core(SP_LOG_DEBUG, log, err, fmt, args);
	va_end(args);
}

#endif