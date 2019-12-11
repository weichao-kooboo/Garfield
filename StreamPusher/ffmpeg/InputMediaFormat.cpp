#include "InputMediaFormat.h"

InputMediaFormat::InputMediaFormat(const string &name) :
	MediaFormat(name, IO_TYPE_INPUT)
{
}

InputMediaFormat::~InputMediaFormat()
{
}

int InputMediaFormat::Open()
{
	int ret = 0;
	unsigned int i;
	if (_name.empty()) {
		return -1;
	}
	if ((ret = checkExtensions()) < 0) {
		return ret;
	}
	AVInputFormat *ifmt = NULL;
	fmt_ctx = NULL;
	const char *local_name = _name.c_str();
	if (InputType == EXTEN_CAMERA) {
		ifmt = av_find_input_format("dshow");
	}
	if ((ret = avformat_open_input(&fmt_ctx, local_name, ifmt, NULL)) < 0) {
		writeLog("Cannot open input file\n");
		return ret;
	}

	if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
		writeLog("Cannot find stream information\n");
		return ret;
	}

	stream_ctx = (StreamContext*)av_mallocz_array(fmt_ctx->nb_streams, sizeof(*stream_ctx));
	if (!stream_ctx)
		return AVERROR(ENOMEM);

	for (i = 0; i < fmt_ctx->nb_streams; i++) {
		AVStream *stream = fmt_ctx->streams[i];
		AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
		AVCodecContext *codec_ctx;
		if (!dec) {
			writeLog("Failed to find decoder for stream #%u\n", i);
			return AVERROR_DECODER_NOT_FOUND;
		}
		codec_ctx = avcodec_alloc_context3(dec);
		if (!codec_ctx) {
			writeLog("Failed to allocate the decoder context for stream #%u\n", i);
			return AVERROR(ENOMEM);
		}
		ret = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
		if (ret < 0) {
			writeLog("Failed to copy decoder parameters to input decoder context "
				"for stream #%u\n", i);
			return ret;
		}
		/* Reencode video & audio and remux subtitles etc. */
		if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
			|| codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
			if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
				codec_ctx->framerate = av_guess_frame_rate(fmt_ctx, stream, NULL);
			/* Open decoder */
			ret = avcodec_open2(codec_ctx, dec, NULL);
			if (ret < 0) {
				writeLog("Failed to open decoder for stream #%u\n", i);
				return ret;
			}
		}
		stream_ctx[i].ctx = codec_ctx;
	}

	parseFormat();

	return 0;
}
