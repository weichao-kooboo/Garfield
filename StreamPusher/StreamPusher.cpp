// StreamPusher.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

extern "C" {
	#include "core/export.h"
}
#include <iostream>

int main(int argc, const char *argv[])
{
	sp_log_t	*log;

	if (sp_strerror_init() != SP_OK) {
		return 1;
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