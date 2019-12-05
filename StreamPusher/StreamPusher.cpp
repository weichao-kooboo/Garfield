﻿// StreamPusher.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "ffmpeg/export.h"
#include <iostream>
#include <functional>

void delete_obj(sp_log_t *log) {
	if (sp_close_file(log->file->fd) == SP_FILE_ERROR) {
		return;
	}
}

int main(int argc, const char *argv[])
{
	using namespace std::placeholders;

	sp_log_t	*log;
	string in_filename, out_filename;

	if (sp_strerror_init() != SP_OK) {
		return 1;
	}
	//sp_log_stderr(0, "this is the tets %s", "hello");
	sp_time_init();

	sp_pid = sp_getpid();

	u_char *p = NULL;
	log = sp_log_init(p);
	//log = (sp_log_t*)malloc(sizeof(sp_log_t));
	if (log == NULL) {
		return 1;
	}
	// log test
	//sp_log_error(SP_LOG_ALERT, log, 0,"start");

	in_filename = "Vi.flv";

	out_filename = "rtmp://192.168.11.138:1935/cctvf";

	avformat_network_init();
	avdevice_register_all();
	RtmpPusher *rp = new RtmpPusher();
	std::shared_ptr<sp_log_t> s_log(log, std::bind(delete_obj, _1));
	//rp->setLogger(std::weak_ptr<sp_log_t>(s_log));
	//rp->push(in_filename,out_filename);
	in_filename = "A4tech USB2.0 Camera";
	MediaFormat *mf = new MediaFormat(in_filename);
	mf->setLogger(std::weak_ptr<sp_log_t>(s_log));
	mf->Open();


	return 0;
}