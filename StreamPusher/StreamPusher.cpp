// StreamPusher.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

extern "C" {
	#include "core/export.h"
}
#define sp_version      1013012
#define SP_VERSION      "1.13.12"
#define SP_VER          "sp/" SP_VERSION

#ifdef SP_BUILD
#define SP_VER_BUILD    SP_VER " (" SP_BUILD ")"
#else
#define SP_VER_BUILD    SP_VER
#endif

#define SP_VAR          "SP"
#define SP_OLDPID_EXT     ".oldbin"

#include <iostream>

static sp_uint_t   sp_show_help;
static sp_uint_t   sp_show_version;
static char        *sp_signal;

static sp_int_t sp_get_options(int argc, char *const *argv);
static void sp_show_version_info(void);

int main(int argc, const char *argv[])
{
	sp_log_t	*log;

	if (sp_strerror_init() != SP_OK) {
		return 1;
	}

	if (sp_show_version) {
		sp_show_version_info();
	}

	//sp_log_stderr(0, "this is the tets %s", "hello");
	sp_time_init();

	sp_pid = sp_getpid();

	u_char *p = NULL;
	log = sp_log_init(p);
	if (log == NULL) {
		return 1;
	}




	return 0;
}

static sp_int_t
sp_get_options(int argc, char *const *argv)
{
	u_char     *p;
	sp_int_t   i;

	for (i = 1; i < argc; i++) {

		p = (u_char *)argv[i];

		if (*p++ != '-') {
			sp_log_stderr(0, "invalid option: \"%s\"", argv[i]);
			return SP_ERROR;
		}

		while (*p) {

			switch (*p++) {

			case '?':
			case 'h':
				sp_show_version = 1;
				sp_show_help = 1;
				break;

			case 'v':
			case 'V':
				sp_show_version = 1;
				break;

			case 's':
				if (*p) {
					sp_signal = (char *)p;

				}
				else if (argv[++i]) {
					sp_signal = argv[i];

				}
				else {
					sp_log_stderr(0, "option \"-s\" requires parameter");
					return SP_ERROR;
				}

				if (sp_strcmp(sp_signal, "stop") == 0
					|| sp_strcmp(sp_signal, "quit") == 0
					|| sp_strcmp(sp_signal, "reopen") == 0
					|| sp_strcmp(sp_signal, "reload") == 0)
				{
					goto next;
				}

				sp_log_stderr(0, "invalid option: \"-s %s\"", sp_signal);
				return SP_ERROR;

			default:
				sp_log_stderr(0, "invalid option: \"%c\"", *(p - 1));
				return SP_ERROR;
			}
		}

	next:

		continue;
	}

	return SP_OK;
}

static void
sp_show_version_info(void)
{
	sp_write_stderr("streamPusher version: " SP_VER_BUILD SP_LINEFEED);

	if (sp_show_help) {
		sp_write_stderr(
			"Usage: streamPusher [-?hvV] [-s signal] " SP_LINEFEED
			SP_LINEFEED
			"Options:" SP_LINEFEED
			"  -?,-h         : this help" SP_LINEFEED
			"  -v            : show version and exit" SP_LINEFEED
			"  -V            : show version and configure options then exit"
			SP_LINEFEED
			"  -s signal     : send signal to a master process: "
			"stop, quit, reopen, reload" SP_LINEFEED
		);
	}
}