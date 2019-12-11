#include "MediaFormat.h"
#include "InputInformation.h"
#include "Logger.h"


MediaFormat::MediaFormat(const string &name,
	const weak_ptr<Logger> &logger,
	IO_Type type)
	:_name(name),
	_type(type),
	_logger(logger),
	_info(new InputInformation())
{
}

MediaFormat::~MediaFormat()
{
	_logger.reset();
	if (_info) {
		if (_info->_duration) {
			free(_info->_duration);
			_info->_duration = NULL;
		}if (_info->_start) {
			free(_info->_start);
			_info->_start = NULL;
		}if (_info->_bitrate) {
			free(_info->_bitrate);
			_info->_bitrate = NULL;
		}
		_info.reset();
	}
	unsigned int i;
	if (!fmt_ctx)
		return;
	for (i = 0; i < fmt_ctx->nb_streams; i++) {
		avcodec_free_context(&stream_ctx[i].ctx);
	}
	av_free(stream_ctx);
	avformat_close_input(&fmt_ctx);
}

int MediaFormat::parseFormat()
{
	int i;
	char str[256];
	memset(str, 0, sizeof(str));
	int is_output = (_type != IO_TYPE_INPUT);

	_info->_format = is_output ? fmt_ctx->oformat->name : fmt_ctx->iformat->name;
	_info->_url = _name.c_str();
	writeLog("%s #, %s, %s '%s':\n",
		is_output ? "Output" : "Input",
		_info->_format,
		is_output ? "to" : "from", _info->_url);

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

			sprintf(str, "Duration:%02d:%02d:%02d.%02d", hours, mins, secs,
				(100 * us) / AV_TIME_BASE);
			_info->_duration = (char*)malloc(sizeof(str));
			memcpy(_info->_duration, str, sizeof(str));
			memset(str, 0, sizeof(str));

			writeLog(_info->_duration);
		}
		if (fmt_ctx->start_time != AV_NOPTS_VALUE) {
			int secs, us;
			secs = llabs(fmt_ctx->start_time / AV_TIME_BASE);
			us = llabs(fmt_ctx->start_time % AV_TIME_BASE);

			sprintf(str, "start:%s%d.%06d", fmt_ctx->start_time >= 0 ? "" : "-",
				secs,
				(int)av_rescale(us, 1000000, AV_TIME_BASE));
			_info->_start = (char*)malloc(sizeof(str));
			memcpy(_info->_start, str, sizeof(str));
			memset(str, 0, sizeof(str));

			writeLog(_info->_start);
		}
		if (fmt_ctx->bit_rate) {
			sprintf(str, "%lld kb/s\n", fmt_ctx->bit_rate / 1000);
			_info->_bitrate = (char*)malloc(sizeof(str));
			memcpy(_info->_bitrate, str, sizeof(str));
			memset(str, 0, sizeof(str));

			writeLog(_info->_bitrate);
		}
	}

	for (i = 0; i < fmt_ctx->nb_streams; i++) {
		parseStreamFormat(i, is_output);
	}
	return 0;
}

void MediaFormat::parseStreamFormat(int i, int is_output)
{
	char buf[256];
	int flags = (is_output ? fmt_ctx->oformat->flags : fmt_ctx->iformat->flags);
	AVStream *st = fmt_ctx->streams[i];
	AVDictionaryEntry *lang = av_dict_get(st->metadata, "language", NULL, 0);
	char *separator = (char *)fmt_ctx->dump_separator;
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
	/*avctx->properties = st->codec->properties;
	avctx->codec = st->codec->codec;
	avctx->qmin = st->codec->qmin;
	avctx->qmax = st->codec->qmax;
	avctx->coded_width = st->codec->coded_width;
	avctx->coded_height = st->codec->coded_height;*/

	if (separator)
		av_opt_set(avctx, "dump_separator", separator, 0);
	avcodec_string(buf, sizeof(buf), avctx, is_output);
	avcodec_free_context(&avctx);

	writeLog("    Stream #%d", i);

	if (flags & AVFMT_SHOW_IDS)
		writeLog("[0x%x]", st->id);
	if (lang)
		writeLog("(%s)", lang->value);
	writeLog(", %d, %d/%d", st->codec_info_nb_frames,
		st->time_base.num, st->time_base.den);
	writeLog(": %s", buf);

	if (st->sample_aspect_ratio.num &&
		av_cmp_q(st->sample_aspect_ratio, st->codecpar->sample_aspect_ratio)) {
		AVRational display_aspect_ratio;
		av_reduce(&display_aspect_ratio.num, &display_aspect_ratio.den,
			st->codecpar->width  * (int64_t)st->sample_aspect_ratio.num,
			st->codecpar->height * (int64_t)st->sample_aspect_ratio.den,
			1024 * 1024);
		writeLog(", SAR %d:%d DAR %d:%d",
			st->sample_aspect_ratio.num, st->sample_aspect_ratio.den,
			display_aspect_ratio.num, display_aspect_ratio.den);
	}

	if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
		int fps = st->avg_frame_rate.den && st->avg_frame_rate.num;
		int tbr = st->r_frame_rate.den && st->r_frame_rate.num;
		int tbn = st->time_base.den && st->time_base.num;
		//int tbc = st->codecpar->time_base.den && st->codec->time_base.num;
		int tbc = 0;

		if (fps || tbr || tbn || tbc)
			writeLog("%s", separator);

		if (fps)
			print_fps(av_q2d(st->avg_frame_rate), tbr || tbn || tbc ? "fps, " : "fps");
		if (tbr)
			print_fps(av_q2d(st->r_frame_rate), tbn || tbc ? "tbr, " : "tbr");
		if (tbn)
			print_fps(1 / av_q2d(st->time_base), tbc ? "tbn, " : "tbn");
		/*if (tbc)
			print_fps(1 / av_q2d(st->codec->time_base), "tbc");*/
	}
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

void MediaFormat::print_fps(double d, const char * postfix)
{
	uint64_t v = lrintf(d * 100);
	if (!v)
		writeLog("%1.4f %s", d, postfix);
	else if (v % 100)
		writeLog("%3.2f %s", d, postfix);
	else if (v % (100 * 1000))
		writeLog("%1.0f %s", d, postfix);
	else
		writeLog("%1.0fk %s", d / 1000, postfix);
}

InputInformation * MediaFormat::getInformation() const
{
	return &(*_info);
}

AVFormatContext * MediaFormat::getFormatContext() const
{
	return fmt_ctx;
}

StreamContext * MediaFormat::getStreamContext() const
{
	return stream_ctx;
}

IO_Type MediaFormat::getType() const
{
	return _type;
}

//这里的...只能接收const char*类型如果使用string类型作为参数会显示为乱码
void MediaFormat::writeLog(const char *fmt, ...)
{
	//todo lock
	shared_ptr<Logger> local_logs = _logger.lock();
	if (!local_logs) {
		sp_log_stderr(0, "logger pointer have been release");
	}
	va_list  args;
	va_start(args, fmt);
	local_logs->writeLog(fmt, args);
	va_end(args);
}
