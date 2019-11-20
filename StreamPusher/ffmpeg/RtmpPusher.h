#pragma once
#ifndef _RTMP_PUSHER_H_INCLUDED_
#define _RTMP_PUSHER_H_INCLUDED_

#include "ffmpegHeader.h"
class RtmpPusher {
public:
	RtmpPusher(sp_log_t *logger);
	~RtmpPusher();
	int push(const char* input_name, const char *output_name);
private:
	int openInput();
	void openOutput();
	void pushStream();
	const char *_input_name;
	const char *_output_name;
	AVFormatContext *_ifmt_ctx,*ofmt_ctx;
	AVOutputFormat *ofmt = NULL;
	sp_log_t *_logger;
};

#endif // !_RTMP_PUSHER_H_INCLUDED_
