#pragma once
#ifndef _MEDIA_FILTER_H_INCLUDED_
#define _MEDIA_FILTER_H_INCLUDED_
#include "ffmpegHeader.h"
class OutputMediaFormat;
class InputMediaFormat;
class Logger;
using namespace std;

typedef struct FilteringContext {
	AVFilterContext *buffersink_ctx;
	AVFilterContext *buffersrc_ctx;
	AVFilterGraph *filter_graph;
} FilteringContext;

class MediaFilter {
public:
	MediaFilter(const weak_ptr<Logger> &logger);
	~MediaFilter();
	int init_filters(InputMediaFormat &input_media_format, OutputMediaFormat &output_media_format);
	void SetFilterSpec(const string &filter_spec);
	FilteringContext *getFilteringCtx() const;
private:
	FilteringContext *_filter_ctx;
	string _filter_spec;
	int init_filter(FilteringContext* fctx, AVCodecContext *dec_ctx,
		AVCodecContext *enc_ctx, const char *filter_spec);
	weak_ptr<Logger> _logger;
	void writeLog(const char * fmt, ...);
};

#endif // !_MEDIA_FILTER_H_INCLUDED_
