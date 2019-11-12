#pragma once
#include "win_files.h"
static sp_int_t sp_win32_check_filename(u_char *name, u_short *u,
	size_t len);
static u_short *sp_utf8_to_utf16(u_short *utf16, u_char *utf8, size_t *len);


ssize_t sp_write_console(sp_fd_t fd, void *buf, size_t size) {
	u_long  n;

	(void)CharToOemBuff(buf, buf, size);

	if (WriteFile(fd, buf, size, &n, NULL) != 0) {
		return (size_t)n;
	}

	return -1;
}

sp_fd_t
sp_open_file(u_char *name, u_long mode, u_long create, u_long access)
{
	size_t      len;
	u_short    *u;
	sp_fd_t    fd;
	sp_err_t   err;
	u_short     utf16[SP_UTF16_BUFLEN];

	len = SP_UTF16_BUFLEN;
	u = sp_utf8_to_utf16(utf16, name, &len);

	if (u == NULL) {
		return INVALID_HANDLE_VALUE;
	}

	fd = INVALID_HANDLE_VALUE;

	if (create == SP_FILE_OPEN
		&& sp_win32_check_filename(name, u, len) != SP_OK)
	{
		goto failed;
	}

	fd = CreateFileW(u, mode,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, create, FILE_FLAG_BACKUP_SEMANTICS, NULL);

failed:

	if (u != utf16) {
		err = sp_errno;
		sp_free(u);
		sp_set_errno(err);
	}

	return fd;
}


static sp_int_t
sp_win32_check_filename(u_char *name, u_short *u, size_t len)
{
	u_char     *p, ch;
	u_long      n;
	u_short    *lu;
	sp_err_t   err;
	enum {
		sw_start = 0,
		sw_normal,
		sw_after_slash,
		sw_after_colon,
		sw_after_dot
	} state;

	/* check for NTFS streams (":"), trailing dots and spaces */

	lu = NULL;
	state = sw_start;

	for (p = name; *p; p++) {
		ch = *p;

		switch (state) {

		case sw_start:

			/*
			 * skip till first "/" to allow paths starting with drive and
			 * relative path, like "c:html/"
			 */

			if (ch == '/' || ch == '\\') {
				state = sw_after_slash;
			}

			break;

		case sw_normal:

			if (ch == ':') {
				state = sw_after_colon;
				break;
			}

			if (ch == '.' || ch == ' ') {
				state = sw_after_dot;
				break;
			}

			if (ch == '/' || ch == '\\') {
				state = sw_after_slash;
				break;
			}

			break;

		case sw_after_slash:

			if (ch == '/' || ch == '\\') {
				break;
			}

			if (ch == '.') {
				break;
			}

			if (ch == ':') {
				state = sw_after_colon;
				break;
			}

			state = sw_normal;
			break;

		case sw_after_colon:

			if (ch == '/' || ch == '\\') {
				state = sw_after_slash;
				break;
			}

			goto invalid;

		case sw_after_dot:

			if (ch == '/' || ch == '\\') {
				goto invalid;
			}

			if (ch == ':') {
				goto invalid;
			}

			if (ch == '.' || ch == ' ') {
				break;
			}

			state = sw_normal;
			break;
		}
	}

	if (state == sw_after_dot) {
		goto invalid;
	}

	/* check if long name match */

	lu = malloc(len * 2);
	if (lu == NULL) {
		return SP_ERROR;
	}

	n = GetLongPathNameW(u, lu, len);

	if (n == 0) {
		goto failed;
	}

	if (n != len - 1 || _wcsicmp(u, lu) != 0) {
		goto invalid;
	}

	sp_free(lu);

	return SP_OK;

invalid:

	sp_set_errno(SP_ENOENT);

failed:

	if (lu) {
		err = sp_errno;
		sp_free(lu);
		sp_set_errno(err);
	}

	return SP_ERROR;
}


static u_short *
sp_utf8_to_utf16(u_short *utf16, u_char *utf8, size_t *len)
{
	u_char    *p;
	u_short   *u, *last;
	uint32_t   n;

	p = utf8;
	u = utf16;
	last = utf16 + *len;

	while (u < last) {

		if (*p < 0x80) {
			*u++ = (u_short)*p;

			if (*p == 0) {
				*len = u - utf16;
				return utf16;
			}

			p++;

			continue;
		}

		if (u + 1 == last) {
			*len = u - utf16;
			break;
		}

		n = sp_utf8_decode(&p, 4);

		if (n > 0x10ffff) {
			sp_set_errno(SP_EILSEQ);
			return NULL;
		}

		if (n > 0xffff) {
			n -= 0x10000;
			*u++ = (u_short)(0xd800 + (n >> 10));
			*u++ = (u_short)(0xdc00 + (n & 0x03ff));
			continue;
		}

		*u++ = (u_short)n;
	}

	/* the given buffer is not enough, allocate a new one */

	u = malloc(((p - utf8) + sp_strlen(p) + 1) * sizeof(u_short));
	if (u == NULL) {
		return NULL;
	}

	sp_memcpy(u, utf16, *len * 2);

	utf16 = u;
	u += *len;

	for (;; ) {

		if (*p < 0x80) {
			*u++ = (u_short)*p;

			if (*p == 0) {
				*len = u - utf16;
				return utf16;
			}

			p++;

			continue;
		}

		n = sp_utf8_decode(&p, 4);

		if (n > 0x10ffff) {
			sp_free(utf16);
			sp_set_errno(SP_EILSEQ);
			return NULL;
		}

		if (n > 0xffff) {
			n -= 0x10000;
			*u++ = (u_short)(0xd800 + (n >> 10));
			*u++ = (u_short)(0xdc00 + (n & 0x03ff));
			continue;
		}

		*u++ = (u_short)n;
	}

	/* unreachable */
}

ssize_t
sp_write_fd(sp_fd_t fd, void *buf, size_t size)
{
	u_long  n;

	if (WriteFile(fd, buf, size, &n, NULL) != 0) {
		return (size_t)n;
	}

	return -1;
}
