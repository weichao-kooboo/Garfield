#pragma once
#include "errno.h"

static sp_str_t *sp_sys_errlist;
static sp_str_t  sp_unknown_error = sp_string("Unknown error");


u_char *sp_strerror(sp_err_t err, u_char *errstr, size_t size) {
	sp_str_t  *msg;

	msg = ((sp_uint_t)err < SP_SYS_NERR) ? &sp_sys_errlist[err] :
		&sp_unknown_error;
	size = min(size, msg->len);

	return sp_cpymem(errstr, msg->data, size);
}

sp_int_t sp_strerror_init(void) {
	char       *msg;
	u_char     *p;
	size_t      len;
	sp_err_t   err;

	len = SP_SYS_NERR * sizeof(sp_str_t);

	sp_sys_errlist = malloc(len);
	if (sp_sys_errlist == NULL) {
		goto failed;
	}

	for (err = 0; err < SP_SYS_NERR; err++) {
		msg = strerror(err);
		//len = ngx_strlen(msg);
		len = strlen(msg);

		p = malloc(len);
		if (p == NULL) {
			goto failed;
		}

		memcpy(p, msg, len);
		sp_sys_errlist[err].len = len;
		sp_sys_errlist[err].data = p;
	}

	return SP_OK;

failed:

	err = errno;
	//ngx_log_stderr(0, "malloc(%uz) failed (%d: %s)", len, err, strerror(err));

	return SP_ERROR;
}