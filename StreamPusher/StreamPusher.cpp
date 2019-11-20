// StreamPusher.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "ffmpeg/export.h"
#include <iostream>

int main(int argc, const char *argv[])
{
	sp_log_t	*log;
	const char *in_filename, *out_filename;

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
	// log test
	sp_log_error(SP_LOG_ALERT, log, 0,
		"start");
	avformat_network_init();
	avdevice_register_all();
	in_filename = "Vi.flv";

	out_filename = "rtmp://192.168.11.138:1935/cctvf";

	RtmpPusher *rp = new RtmpPusher(log);
	rp->push(in_filename,out_filename);
	return 0;
}