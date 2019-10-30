#pragma once
#include "win_errno.h"

static sp_str_t *sp_sys_errlist;
static sp_str_t  sp_unknown_error = sp_string("Unknown error");


u_char *sp_strerror(sp_err_t err, u_char *errstr, size_t size) {
	u_int          len;
	static u_long  lang = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

	if (size == 0) {
		return errstr;
	}

	len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err, lang, (char *)errstr, size, NULL);

	if (len == 0 && lang && GetLastError() == ERROR_RESOURCE_LANG_NOT_FOUND) {
		lang = 0;

		len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, err, lang, (char *)errstr, size, NULL);
	}

	if (len == 0) {
		//return ngx_snprintf(errstr, size,
		//	"FormatMessage() error:(%d)", GetLastError());
	}

	/* remove ".\r\n\0" */
	while (errstr[len] == '\0' || errstr[len] == CR
		|| errstr[len] == LF || errstr[len] == '.')
	{
		--len;
	}

	return &errstr[++len];
}

sp_int_t sp_strerror_init(void) {
	return SP_OK;
}