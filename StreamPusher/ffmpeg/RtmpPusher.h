#pragma once
#ifndef _RTMP_PUSHER_H_INCLUDED_
#define _RTMP_PUSHER_H_INCLUDED_

#include "ffmpegHeader.h"
using namespace std;

typedef std::weak_ptr<sp_log_t> wpLog;
typedef std::shared_ptr<sp_log_t> spLog;

class RtmpPusher {
public:
	RtmpPusher();
	~RtmpPusher();
	int push(string &input_name, string &output_name);
	void setLogger(std::weak_ptr<sp_log_t> logger);
private:
	int openInput();
	int openOutput();
	int pushStream();
	void writeLog(const char *fmt, ...);
	int videoindex;
	string _input_name;
	string _output_name;
	AVFormatContext *ifmt_ctx,*ofmt_ctx;
	AVOutputFormat *ofmt = NULL;
	wpLog _logger;
};

#endif // !_RTMP_PUSHER_H_INCLUDED_
