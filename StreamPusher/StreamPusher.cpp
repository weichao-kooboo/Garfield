// StreamPusher.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

extern "C" {
	#include "core/export.h"
}
#include <iostream>

int main(int argc, const char *argv[])
{
	sp_strerror_init();
	/*u_char    errstr[1024];
	u_char   *p;
	p = sp_cpymem(errstr, "streamPusher: ", 14);
	(void)sp_write_console(sp_stderr, errstr, p - errstr);*/
	return 0;
}