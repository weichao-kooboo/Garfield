#pragma once
#ifndef _MEDIA_FILTER_H_INCLUDED_
#define _MEDIA_FILTER_H_INCLUDED_
#include "ffmpegHeader.h"
class OutputMediaFormat;
using namespace std;
class MediaFilter {
public:
	MediaFilter(const weak_ptr<OutputMediaFormat> &output_media_format);
	~MediaFilter();
	int init_filters(void);
	void SetFilterSpec(const string &filter_spec);
private:
	typedef struct FilteringContext {
		AVFilterContext *buffersink_ctx;
		AVFilterContext *buffersrc_ctx;
		AVFilterGraph *filter_graph;
	} FilteringContext;
	FilteringContext *_filter_ctx;
	string _filter_spec;
	int init_filter(FilteringContext* fctx, AVCodecContext *dec_ctx,
		AVCodecContext *enc_ctx, const char *filter_spec);
	weak_ptr<OutputMediaFormat> _output_media_format;
};

#endif // !_MEDIA_FILTER_H_INCLUDED_
