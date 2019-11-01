// StreamPusher.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

extern "C" {
	#include "core/export.h"
}
#include <iostream>

int main(int argc, const char *argv[])
{
	sp_strerror_init();
	sp_log_stderr(0, "this is the tets %s", "hello");
	return 0;
}