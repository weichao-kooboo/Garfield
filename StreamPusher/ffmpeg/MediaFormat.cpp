#include "MediaFormat.h"
#include "InputInformation.h"


MediaFormat::MediaFormat(const string &name,
	IO_Type type)
	:_name(name),
	_type(type),
	_info(new InputInformation())
{
}

MediaFormat::~MediaFormat()
{
	_logger.reset();
	_info.reset();
	unsigned int i;
	if (!fmt_ctx)
		return;
	for (i = 0; i < fmt_ctx->nb_streams; i++) {
		avcodec_free_context(&stream_ctx[i].ctx);
	}
	av_free(stream_ctx);
	avformat_close_input(&fmt_ctx);
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

	parseFormat();

	return 0;
}

int MediaFormat::parseFormat()
{
	int i;
	int is_output = (_type == IO_TYPE_INPUT);
	writeLog("%s #, %s, %s '%s':\n",
		is_output ? "Output" : "Input",
		is_output ? fmt_ctx->oformat->name : fmt_ctx->iformat->name,
		is_output ? "to" : "from", _name);

	if (!is_output) {
		//打印持续时间Duration
		if (fmt_ctx->duration != AV_NOPTS_VALUE) {
			int hours, mins, secs, us;
			int64_t duration = fmt_ctx->duration + (fmt_ctx->duration <= INT64_MAX - 5000 ? 5000 : 0);
			secs = duration / AV_TIME_BASE;
			us = duration % AV_TIME_BASE;
			mins = secs / 60;
			secs %= 60;
			hours = mins / 60;
			mins %= 60;
			writeLog("Duration:%02d:%02d:%02d.%02d", hours, mins, secs,
				(100 * us) / AV_TIME_BASE);
		}
		if (fmt_ctx->start_time != AV_NOPTS_VALUE) {
			int secs, us;
			secs = llabs(fmt_ctx->start_time / AV_TIME_BASE);
			us = llabs(fmt_ctx->start_time % AV_TIME_BASE);
			writeLog("start:%s%d.%06d",
				fmt_ctx->start_time >= 0 ? "" : "-",
				secs,
				(int)av_rescale(us, 1000000, AV_TIME_BASE));
		}
		if (fmt_ctx->bit_rate)
			writeLog("%lld kb/s\n", fmt_ctx->bit_rate / 1000);
	}

	for (i = 0; i < fmt_ctx->nb_streams; i++) {
		parseStreamFormat(i, is_output);
	}
	return 0;
}

int MediaFormat::parseStreamFormat(int i, int is_output)
{
	char buf[256];
	int flags = (is_output ? fmt_ctx->oformat->flags : fmt_ctx->iformat->flags);
	AVStream *st = fmt_ctx->streams[i];
	AVDictionaryEntry *lang = av_dict_get(st->metadata, "language", NULL, 0);
	char *separator = fmt_ctx->dump_separator;
	AVCodecContext *avctx;
	int ret;

	avctx = avcodec_alloc_context3(NULL);
	if (!avctx)
		return;

	ret = avcodec_parameters_to_context(avctx, st->codecpar);
	if (ret < 0) {
		avcodec_free_context(&avctx);
		return;
	}

	// Fields which are missing from AVCodecParameters need to be taken from the AVCodecContext
	avctx->properties = st->codec->properties;
	avctx->codec = st->codec->codec;
	avctx->qmin = st->codec->qmin;
	avctx->qmax = st->codec->qmax;
	avctx->coded_width = st->codec->coded_width;
	avctx->coded_height = st->codec->coded_height;

	if (separator)
		av_opt_set(avctx, "dump_separator", separator, 0);
	avcodec_string(buf, sizeof(buf), avctx, is_output);
	avcodec_free_context(&avctx);

	av_log(NULL, AV_LOG_INFO, "    Stream #%d:%d", index, i);

	if (flags & AVFMT_SHOW_IDS)
		av_log(NULL, AV_LOG_INFO, "[0x%x]", st->id);
	if (lang)
		av_log(NULL, AV_LOG_INFO, "(%s)", lang->value);
	av_log(NULL, AV_LOG_DEBUG, ", %d, %d/%d", st->codec_info_nb_frames,
		st->time_base.num, st->time_base.den);
	av_log(NULL, AV_LOG_INFO, ": %s", buf);

	if (st->sample_aspect_ratio.num &&
		av_cmp_q(st->sample_aspect_ratio, st->codecpar->sample_aspect_ratio)) {
		AVRational display_aspect_ratio;
		av_reduce(&display_aspect_ratio.num, &display_aspect_ratio.den,
			st->codecpar->width  * (int64_t)st->sample_aspect_ratio.num,
			st->codecpar->height * (int64_t)st->sample_aspect_ratio.den,
			1024 * 1024);
		av_log(NULL, AV_LOG_INFO, ", SAR %d:%d DAR %d:%d",
			st->sample_aspect_ratio.num, st->sample_aspect_ratio.den,
			display_aspect_ratio.num, display_aspect_ratio.den);
	}

	if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
		int fps = st->avg_frame_rate.den && st->avg_frame_rate.num;
		int tbr = st->r_frame_rate.den && st->r_frame_rate.num;
		int tbn = st->time_base.den && st->time_base.num;
		int tbc = st->codec->time_base.den && st->codec->time_base.num;

		if (fps || tbr || tbn || tbc)
			av_log(NULL, AV_LOG_INFO, "%s", separator);

		if (fps)
			print_fps(av_q2d(st->avg_frame_rate), tbr || tbn || tbc ? "fps, " : "fps");
		if (tbr)
			print_fps(av_q2d(st->r_frame_rate), tbn || tbc ? "tbr, " : "tbr");
		if (tbn)
			print_fps(1 / av_q2d(st->time_base), tbc ? "tbn, " : "tbn");
		if (tbc)
			print_fps(1 / av_q2d(st->codec->time_base), "tbc");
	}
	return 0;
}

int MediaFormat::checkExtensions()
{
	if (_name.empty()) {
		return -1;
	}
	string fileName = _name;
	size_t exten_len = -1;
	if ((exten_len = fileName.find('.')) < 0
		|| fileName.find("USB") >= 0) {
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

InputInformation * MediaFormat::getInformation() const
{
	return &(*_info);
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
