#pragma once
#include "sp_string.h"

static u_char *sp_sprintf_num(u_char *buf, u_char *last, uint64_t ui64,
	u_char zero, sp_uint_t hexadecimal, sp_uint_t width);


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


static u_char *
sp_sprintf_num(u_char *buf, u_char *last, uint64_t ui64, u_char zero,
	sp_uint_t hexadecimal, sp_uint_t width)
{
	u_char         *p, temp[SP_INT64_LEN + 1];
	/*
	 * we need temp[SP_INT64_LEN] only,
	 * but icc issues the warning
	 */
	size_t          len;
	uint32_t        ui32;
	static u_char   hex[] = "0123456789abcdef";
	static u_char   HEX[] = "0123456789ABCDEF";

	p = temp + SP_INT64_LEN;

	if (hexadecimal == 0) {

		if (ui64 <= (uint64_t)SP_MAX_UINT32_VALUE) {

			/*
			 * To divide 64-bit numbers and to find remainders
			 * on the x86 platform gcc and icc call the libc functions
			 * [u]divdi3() and [u]moddi3(), they call another function
			 * in its turn.  On FreeBSD it is the qdivrem() function,
			 * its source code is about 170 lines of the code.
			 * The glibc counterpart is about 150 lines of the code.
			 *
			 * For 32-bit numbers and some divisors gcc and icc use
			 * a inlined multiplication and shifts.  For example,
			 * unsigned "i32 / 10" is compiled to
			 *
			 *     (i32 * 0xCCCCCCCD) >> 35
			 */

			ui32 = (uint32_t)ui64;

			do {
				*--p = (u_char)(ui32 % 10 + '0');
			} while (ui32 /= 10);

		}
		else {
			do {
				*--p = (u_char)(ui64 % 10 + '0');
			} while (ui64 /= 10);
		}

	}
	else if (hexadecimal == 1) {

		do {

			/* the "(uint32_t)" cast disables the BCC's warning */
			*--p = hex[(uint32_t)(ui64 & 0xf)];

		} while (ui64 >>= 4);

	}
	else { /* hexadecimal == 2 */

		do {

			/* the "(uint32_t)" cast disables the BCC's warning */
			*--p = HEX[(uint32_t)(ui64 & 0xf)];

		} while (ui64 >>= 4);
	}

	/* zero or space padding */

	len = (temp + SP_INT64_LEN) - p;

	while (len++ < width && buf < last) {
		*buf++ = zero;
	}

	/* number safe copy */

	len = (temp + SP_INT64_LEN) - p;

	if (buf + len > last) {
		len = last - buf;
	}

	return sp_cpymem(buf, p, len);
}


u_char * sp_cdecl
sp_slprintf(u_char *buf, u_char *last, const char *fmt, ...)
{
	u_char   *p;
	va_list   args;

	va_start(args, fmt);
	p = sp_vslprintf(buf, last, fmt, args);
	va_end(args);

	return p;
}

u_char * sp_cdecl
sp_snprintf(u_char *buf, size_t max, const char *fmt, ...)
{
	u_char   *p;
	va_list   args;

	va_start(args, fmt);
	p = sp_vslprintf(buf, buf + max, fmt, args);
	va_end(args);

	return p;
}


uint32_t
sp_utf8_decode(u_char **p, size_t n)
{
	size_t    len;
	uint32_t  u, i, valid;

	u = **p;

	if (u >= 0xf0) {

		u &= 0x07;
		valid = 0xffff;
		len = 3;

	}
	else if (u >= 0xe0) {

		u &= 0x0f;
		valid = 0x7ff;
		len = 2;

	}
	else if (u >= 0xc2) {

		u &= 0x1f;
		valid = 0x7f;
		len = 1;

	}
	else {
		(*p)++;
		return 0xffffffff;
	}

	if (n - 1 < len) {
		return 0xfffffffe;
	}

	(*p)++;

	while (len) {
		i = *(*p)++;

		if (i < 0x80) {
			return 0xffffffff;
		}

		u = (u << 6) | (i & 0x3f);

		len--;
	}

	if (u > valid) {
		return u;
	}

	return 0xffffffff;
}