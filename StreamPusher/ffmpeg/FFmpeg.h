#pragma once
#ifndef _SP_FFMPEG_H_INCLUDED_
#define	_SP_FFMPEG_H_INCLUDED_
#include "ffmpegHeader.h"
#include "MediaFilter.h";
#include <functional>
class InputMediaFormat;
class OutputMediaFormat;
class Logger;
using namespace std;

class FFmpeg {
public:
	FFmpeg(const shared_ptr<MediaFilter> &mediaFilter, const weak_ptr<Logger> &logger);
	~FFmpeg();
	int start();
private:
	void readFrameLoop(shared_ptr<FilteringContext> &_filter_ctx,
		shared_ptr<OutputMediaFormat> &_output_format,
		shared_ptr<InputMediaFormat> &_input_format);
	shared_ptr<MediaFilter> _mediaFilter;
	int encodeCallBack(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame, unsigned int stream_index);
	int decodeCallBack(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame, unsigned int stream_index);
	int filterEncodeWriteFrame(AVFrame *frame, unsigned int stream_index);
	weak_ptr<Logger> _logger;
	void writeLog(const char * fmt, ...);
};

#endif // !_SP_FFMPEG_H_INCLUDED_
