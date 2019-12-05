#include "MediaFormat.h"

enum {
	EXTEN_CAMERA,
	EXTEN_FILE
}InputType;

MediaFormat::MediaFormat(const string &name)
{
	_name = name;
}

MediaFormat::~MediaFormat()
{
}

int MediaFormat::Open()
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

	return 0;
}

int MediaFormat::parseFormat()
{
	return 0;
}

int MediaFormat::checkExtensions()
{
	if (_name.empty()) {
		return -1;
	}
	string fileName = _name;
	size_t exten_len = -1;
	if ((exten_len = fileName.find('.')) < 0) {
		InputType = EXTEN_CAMERA;
		return 0;
	}
	string extensions = fileName.substr(exten_len, fileName.size());
	const AVInputFormat *fmt = NULL;
	void *i = 0;
	while ((fmt = av_demuxer_iterate(&i))) {
		if (av_match_name(extensions.c_str(), fmt->extensions)) {
			InputType = EXTEN_FILE;
			return 0;
		}
	}
	
	return -1;
}

InputInformation * MediaFormat::getInformation()
{
	return nullptr;
}

void MediaFormat::setLogger(std::weak_ptr<sp_log_t> logger)
{
	std::weak_ptr<sp_log_t> local_log;
	{
		//todo:lock log obj
		_logger.swap(local_log);
		_logger = logger;
	}
	local_log.reset();
}

//这里的...只能接收const char*类型如果使用string类型作为参数会显示为乱码
void MediaFormat::writeLog(const char *fmt, ...)
{
	//todo lock
	spLog local_logs = _logger.lock();
	if (!local_logs) {
		sp_log_stderr(0, "logger pointer have been release");
	}
	sp_log_t* origin_ptr = &(*local_logs);
	va_list  args;
	if (origin_ptr->log_level >= SP_LOG_ALERT) {
		va_start(args, fmt);
		sp_log_error_msg(SP_LOG_ALERT, origin_ptr, 0, fmt, args);
		va_end(args);
	}
}
