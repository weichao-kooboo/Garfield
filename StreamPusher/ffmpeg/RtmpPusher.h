#pragma once
#ifndef _RTMP_PUSHER_H_INCLUDED_
#define _RTMP_PUSHER_H_INCLUDED_

#include "ffmpegHeader.h"
using namespace std;

typedef std::weak_ptr<string> wpString;
typedef std::weak_ptr<sp_log_t> wpLog;
typedef std::shared_ptr<sp_log_t> spLog;

class RtmpPusher {
public:
	RtmpPusher();
	~RtmpPusher();
	int push(const char* input_name, const char *output_name);
	void setLogger(std::weak_ptr<sp_log_t> logger);
private:
	int openInput();
	void openOutput();
	void pushStream();
	void writeLog(const char *fmt, ...);
	wpString _input_name;
	wpString _output_name;
	AVFormatContext *_ifmt_ctx,*ofmt_ctx;
	AVOutputFormat *ofmt = NULL;
	wpLog _logger;
};

#endif // !_RTMP_PUSHER_H_INCLUDED_
