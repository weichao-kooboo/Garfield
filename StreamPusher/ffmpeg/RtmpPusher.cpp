#include "RtmpPusher.h"

RtmpPusher::RtmpPusher()
{
}

RtmpPusher::~RtmpPusher()
{

}

int RtmpPusher::push(string &input_name, string &output_name)
{
	if (input_name.empty() || output_name.empty()) {
		return -1;
	}

	_input_name = input_name;
	_output_name = output_name;
	int ret = 0;
	if ((ret = openInput()) < 0) {
		goto end;
	}
	if ((ret = openOutput()) < 0) {
		goto end;
	}
	if ((ret = pushStream()) < 0) {
		goto end;
	}

end:
	avformat_close_input(&ifmt_ctx);
	if (ofmt_ctx && !(ofmt->flags&AVFMT_NOFILE)) {
		avio_close(ofmt_ctx->pb);
	}
	avformat_free_context(ofmt_ctx);
	if (ret < 0 && ret != AVERROR_EOF) {
		sp_log_stderr(0, "Error occurred.\n");
		writeLog("Error occurred.\n");
		return -1;
	}
}

void RtmpPusher::setLogger(std::weak_ptr<sp_log_t> logger)
{
	std::weak_ptr<sp_log_t> local_log;
	{
		//todo:lock log obj
		_logger.swap(local_log);
		_logger = logger;
	}
	local_log.reset();
}

int RtmpPusher::openInput()
{
	int ret = 0;
	int i = 0;
	if (_input_name.empty()) {
		writeLog(" input name:%s have been release", _input_name.c_str());
	}

#if USE_DSHOW
	AVInputFormat *ifmt = av_find_input_format("dshow");
	_input_name = "XiaoMi USB 2.0 Webcam";
	if ((ret = avformat_open_input(&ifmt_ctx, _input_name.c_str(), ifmt, 0)) < 0)
#else
	if ((ret = avformat_open_input(&ifmt_ctx, _input_name.c_str(), 0, 0)) < 0)
#endif
	 {
		writeLog(" open:%s failed", _input_name.c_str());
		return ret;
	}

	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		writeLog("Failed to retrieve input stream information");
		return ret;
	}
	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
			break;
		}
	}
	av_dump_format(ifmt_ctx, 0, _input_name.c_str(), 0);
	return 0;
}

int RtmpPusher::openOutput()
{
	int ret = 0;
	int i = 0;
	if (_output_name.empty()) {
		writeLog(" input name:%s have been release", _output_name.c_str());
	}

	avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", _output_name.c_str());

	if (!ofmt_ctx) {
		writeLog("Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		return ret;
	}
	ofmt = ofmt_ctx->oformat;
	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		AVStream *in_stream = ifmt_ctx->streams[i];

		AVCodec *pLocalCodec = NULL;
		pLocalCodec = avcodec_find_decoder(in_stream->codecpar->codec_id);
		AVStream *out_stream = avformat_new_stream(ofmt_ctx, pLocalCodec);
		if (!out_stream) {
			writeLog("Failed allocating output stream\n");
			ret = AVERROR_UNKNOWN;
			return ret;
		}

		AVCodecContext *codec_ctx = avcodec_alloc_context3(pLocalCodec);
		ret = avcodec_parameters_to_context(codec_ctx, in_stream->codecpar);
		if (ret < 0) {
			writeLog("Failed to copy in_stream codecpar to codec context\n");
			return ret;
		}
		codec_ctx->codec_tag = 0;
		if (ofmt_ctx->oformat->flags&AVFMT_GLOBALHEADER) {
			codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}
		ret = avcodec_parameters_from_context(out_stream->codecpar, codec_ctx);
		if (ret < 0) {
			writeLog("Failed to copy codec context to out_stream codecpar context\n");
			return ret;
		}
	}

	av_dump_format(ofmt_ctx, 0, _output_name.c_str(), 1);
}

int RtmpPusher::pushStream()
{
	AVPacket pkt;
	int ret = 0;
	int frame_index = 0;
	int64_t start_time = 0;
	if (!(ofmt->flags&AVFMT_NOFILE)) {
		ret = avio_open(&ofmt_ctx->pb, _output_name.c_str(), AVIO_FLAG_WRITE);
		if (ret < 0) {
			writeLog("Could not open output URL '%s'", _output_name.c_str());
			return ret;
		}
	}
	ret = avformat_write_header(ofmt_ctx, NULL);
	if (ret < 0) {
		writeLog("Error occurred when opening output URL\n");
		return ret;
	}
	start_time = av_gettime();
	while (1) {
		AVStream *in_stream, *out_stream;

		ret = av_read_frame(ifmt_ctx, &pkt);
		if (ret < 0) {
			break;
		}
		if (pkt.pts == AV_NOPTS_VALUE) {
			AVRational time_base1 = ifmt_ctx->streams[videoindex]->time_base;
			int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(ifmt_ctx->streams[videoindex]->r_frame_rate);
			//Parameters
			pkt.pts = (double)(frame_index*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
			pkt.dts = pkt.pts;
			pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
		}
		//Important:Delay
		if (pkt.stream_index == videoindex) {
			AVRational time_base = ifmt_ctx->streams[videoindex]->time_base;
			AVRational time_base_q = { 1,AV_TIME_BASE };
			int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
			int64_t now_time = av_gettime() - start_time;
			if (pts_time > now_time)
				av_usleep(pts_time - now_time);

		}

		in_stream = ifmt_ctx->streams[pkt.stream_index];
		out_stream = ofmt_ctx->streams[pkt.stream_index];
		// copy packet
		// 转换PTS/DTS（Convert PTS/DTS）
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		//Print to Screen
		if (pkt.stream_index == videoindex) {
			writeLog("Send %8d video frames to output URL\n", frame_index);
			frame_index++;
		}
		//ret = av_write_frame(ofmt_ctx, &pkt);
		ret = av_interleaved_write_frame(ofmt_ctx, &pkt);

		if (ret < 0) {
			writeLog("Error muxing packet\n");
			break;
		}

		av_packet_unref(&pkt);
	}
	av_write_trailer(ofmt_ctx);
}

//这里的...只能接收const char*类型如果使用string类型作为参数会显示为乱码
void RtmpPusher::writeLog(const char *fmt, ...)
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
