#pragma once
#include "sp_string.h"


u_char *
sp_vslprintf(u_char *buf, u_char *last, const char *fmt, va_list args)
{
	u_char                *p, zero;
	int                    d;
	double                 f;
	size_t                 len, slen;
	int64_t                i64;
	uint64_t               ui64, frac;
	sp_msec_t             ms;
	sp_uint_t             width, sign, hex, max_width, frac_width, scale, n;
	sp_str_t             *v;
	sp_variable_value_t  *vv;

	while (*fmt && buf < last) {

		/*
		 * "buf < last" means that we could copy at least one character:
		 * the plain character, "%%", "%c", and minus without the checking
		 */

		if (*fmt == '%') {

			i64 = 0;
			ui64 = 0;

			zero = (u_char)((*++fmt == '0') ? '0' : ' ');
			width = 0;
			sign = 1;
			hex = 0;
			max_width = 0;
			frac_width = 0;
			slen = (size_t)-1;

			while (*fmt >= '0' && *fmt <= '9') {
				width = width * 10 + (*fmt++ - '0');
			}


			for (;; ) {
				switch (*fmt) {

				case 'u':
					sign = 0;
					fmt++;
					continue;

				case 'm':
					max_width = 1;
					fmt++;
					continue;

				case 'X':
					hex = 2;
					sign = 0;
					fmt++;
					continue;

				case 'x':
					hex = 1;
					sign = 0;
					fmt++;
					continue;

				case '.':
					fmt++;

					while (*fmt >= '0' && *fmt <= '9') {
						frac_width = frac_width * 10 + (*fmt++ - '0');
					}

					break;

				case '*':
					slen = va_arg(args, size_t);
					fmt++;
					continue;

				default:
					break;
				}

				break;
			}


			switch (*fmt) {

			case 'V':
				v = va_arg(args, sp_str_t *);

				len = sp_min(((size_t)(last - buf)), v->len);
				buf = sp_cpymem(buf, v->data, len);
				fmt++;

				continue;

			case 'v':
				vv = va_arg(args, sp_variable_value_t *);

				len = sp_min(((size_t)(last - buf)), vv->len);
				buf = sp_cpymem(buf, vv->data, len);
				fmt++;

				continue;

			case 's':
				p = va_arg(args, u_char *);

				if (slen == (size_t)-1) {
					while (*p && buf < last) {
						*buf++ = *p++;
					}

				}
				else {
					len = sp_min(((size_t)(last - buf)), slen);
					buf = sp_cpymem(buf, p, len);
				}

				fmt++;

				continue;

			case 'O':
				i64 = (int64_t)va_arg(args, sp_off_t);
				sign = 1;
				break;

			case 'P':
				i64 = (int64_t)va_arg(args, sp_pid_t);
				sign = 1;
				break;

			case 'T':
				i64 = (int64_t)va_arg(args, time_t);
				sign = 1;
				break;

			case 'M':
				ms = (sp_msec_t)va_arg(args, sp_msec_t);
				if ((sp_msec_int_t)ms == -1) {
					sign = 1;
					i64 = -1;
				}
				else {
					sign = 0;
					ui64 = (uint64_t)ms;
				}
				break;

			case 'z':
				if (sign) {
					i64 = (int64_t)va_arg(args, ssize_t);
				}
				else {
					ui64 = (uint64_t)va_arg(args, size_t);
				}
				break;

			case 'i':
				if (sign) {
					i64 = (int64_t)va_arg(args, sp_int_t);
				}
				else {
					ui64 = (uint64_t)va_arg(args, sp_uint_t);
				}

				if (max_width) {
					width = SP_INT_T_LEN;
				}

				break;

			case 'd':
				if (sign) {
					i64 = (int64_t)va_arg(args, int);
				}
				else {
					ui64 = (uint64_t)va_arg(args, u_int);
				}
				break;

			case 'l':
				if (sign) {
					i64 = (int64_t)va_arg(args, long);
				}
				else {
					ui64 = (uint64_t)va_arg(args, u_long);
				}
				break;

			case 'D':
				if (sign) {
					i64 = (int64_t)va_arg(args, int32_t);
				}
				else {
					ui64 = (uint64_t)va_arg(args, uint32_t);
				}
				break;

			case 'L':
				if (sign) {
					i64 = va_arg(args, int64_t);
				}
				else {
					ui64 = va_arg(args, uint64_t);
				}
				break;

			case 'A':
				if (sign) {
					i64 = (int64_t)va_arg(args, sp_atomic_int_t);
				}
				else {
					ui64 = (uint64_t)va_arg(args, sp_atomic_uint_t);
				}

				if (max_width) {
					width = SP_ATOMIC_T_LEN;
				}

				break;

			case 'f':
				f = va_arg(args, double);

				if (f < 0) {
					*buf++ = '-';
					f = -f;
				}

				ui64 = (int64_t)f;
				frac = 0;

				if (frac_width) {

					scale = 1;
					for (n = frac_width; n; n--) {
						scale *= 10;
					}

					frac = (uint64_t)((f - (double)ui64) * scale + 0.5);

					if (frac == scale) {
						ui64++;
						frac = 0;
					}
				}

				buf = sp_sprintf_num(buf, last, ui64, zero, 0, width);

				if (frac_width) {
					if (buf < last) {
						*buf++ = '.';
					}

					buf = sp_sprintf_num(buf, last, frac, '0', 0, frac_width);
				}

				fmt++;

				continue;

			#if !(SP_WIN32)
			case 'r':
				i64 = (int64_t)va_arg(args, rlim_t);
				sign = 1;
				break;
			#endif

			case 'p':
				ui64 = (uintptr_t)va_arg(args, void *);
				hex = 2;
				sign = 0;
				zero = '0';
				width = 2 * sizeof(void *);
				break;

			case 'c':
				d = va_arg(args, int);
				*buf++ = (u_char)(d & 0xff);
				fmt++;

				continue;

			case 'Z':
				*buf++ = '\0';
				fmt++;

				continue;

			case 'N':
#if (SP_WIN32)
				*buf++ = CR;
				if (buf < last) {
					*buf++ = LF;
				}
#else
				*buf++ = LF;
#endif
				fmt++;

				continue;

			case '%':
				*buf++ = '%';
				fmt++;

				continue;

			default:
				*buf++ = *fmt++;

				continue;
			}

			if (sign) {
				if (i64 < 0) {
					*buf++ = '-';
					ui64 = (uint64_t)-i64;

				}
				else {
					ui64 = (uint64_t)i64;
				}
			}

			buf = sp_sprintf_num(buf, last, ui64, zero, hex, width);

			fmt++;

		}
		else {
			*buf++ = *fmt++;
		}
	}

	return buf;
}