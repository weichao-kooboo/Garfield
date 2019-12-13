#pragma once
#ifndef _SP_FFMPEG_H_INCLUDED_
#define	_SP_FFMPEG_H_INCLUDED_
#include "ffmpegHeader.h"
#include "MediaFilter.h";
#include "InputMediaFormat.h";
#include "OutputMediaFormat.h";
class Logger;
using namespace std;

class FFmpeg {
public:
	FFmpeg(const weak_ptr<Logger> &logger);
	~FFmpeg();
	int start(InputMediaFormat &input_media_format, OutputMediaFormat &output_media_format, MediaFilter &_filter_ctx);
private:
	int readFrameLoop();
	int encodeCallBack(AVCodecContext *pCodecContext, AVFrame *pFrame, AVPacket *pPacket, unsigned int stream_index);
	int decodeCallBack(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame, unsigned int stream_index);
	int filterDecodeReadFrame(AVFrame *frame, unsigned int stream_index);
	int encodeWriteFrame(AVFrame *filt_frame, unsigned int stream_index);
	int flushEncoder(unsigned int stream_index);
	AVFormatContext *ifmt_ctx;
	StreamContext *istream_ctx;
	AVFormatContext *ofmt_ctx;
	StreamContext *ostream_ctx;
	FilteringContext *filter_ctx;
	weak_ptr<Logger> _logger;
	void writeLog(const char * fmt, ...);
};

#endif // !_SP_FFMPEG_H_INCLUDED_
