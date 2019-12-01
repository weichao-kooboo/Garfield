// test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "ShowDevices.h"
#include "AvioReading.h"

AVInputFormat *find_input_format(const char *short_name) {
	const AVInputFormat *fmt = NULL;
	void *i = 0;
	while ((fmt = av_demuxer_iterate(&i))) {
		printf("%s \n",fmt->name);
		if (av_match_name(short_name, fmt->name)) {
			return (AVInputFormat*)fmt;
		}
	}
	return NULL;
}

/* 解析视频流里面的每一帧
 * 参考:https://github.com/leandromoreira/ffmpeg-libav-tutorial#intro
 */
extern "C" {
	static void logging(const char *fmt, ...);

	static int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame);

	static void save_gray_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename);
}

int parseFrame(int argc, const char *argv[])
{
	logging("initializing all the containers,codecs and protocols");

	const char *in_filename = "Vi.flv";

	AVFormatContext *pFormatContext = avformat_alloc_context();
	if (!pFormatContext) {
		logging("ERROR could not allocate memory for Format Context");
		return -1;
	}

	logging("opening the input file (%s) and loading format (container) header", in_filename);

	if (avformat_open_input(&pFormatContext, in_filename, NULL, NULL) != 0) {
		logging("ERROR could not open the file");
		return -1;
	}

	logging("format %s,duration %lld us,but_rate %lld", pFormatContext->iformat->name, pFormatContext->duration, pFormatContext->bit_rate);

	logging("finding stream info from format");

	if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
		logging("ERROR could not get the stream info");
		return -1;
	}
	// 计算时长
	// 必须在调用函数avformat_find_stream_info之后,否则是无效值
	//https://blog.csdn.net/leixiaohua1020/article/details/14214705
	int tns, thh, tmm, tss;
	tns = (pFormatContext->duration) / 1000000;
	thh = tns / 3600;
	tmm = (tns % 3600) / 60;
	tss = (tns % 60);

	logging("duration %02d:%02d:%02d", thh, tmm, tss);
	av_dump_format(pFormatContext, 0, in_filename, 0);

	AVCodec *pCodec = NULL;
	AVCodecParameters *pCodecParameters = NULL;
	int video_stream_index = -1;

	for (int i = 0; i < pFormatContext->nb_streams; i++) {
		AVCodecParameters *pLocalCodecParameters = NULL;
		pLocalCodecParameters = pFormatContext->streams[i]->codecpar; 
		logging("AVStream->time_base before open coded %d/%d", pFormatContext->streams[i]->time_base.num, pFormatContext->streams[i]->time_base.den);
		logging("AVStream->r_frame_rate before open coded %d/%d", pFormatContext->streams[i]->r_frame_rate.num, pFormatContext->streams[i]->r_frame_rate.den);
		logging("AVStream->start_time %" PRId64, pFormatContext->streams[i]->start_time);
		logging("AVStream->duration %" PRId64, pFormatContext->streams[i]->duration);

		logging("finding the proper decoder (CODEC)");

		AVCodec *pLocalCodec = NULL;

		pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

		if (pLocalCodec == NULL) {
			logging("ERROR unsupported codec!");
			return -1;
		}

		if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
			if (video_stream_index == -1) {
				video_stream_index = i;
				pCodec = pLocalCodec;
				pCodecParameters = pLocalCodecParameters;
			}
			logging("Video Codec: resolution %d x %d", pLocalCodecParameters->width, pLocalCodecParameters->height);
		}
		else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
			logging("Audio Codec: %d channels, sample rate %d", pLocalCodecParameters->channels, pLocalCodecParameters->sample_rate);
		}
		logging("\tCodec %s ID %d bit_rate %lld", pLocalCodec->name, pLocalCodec->id, pCodecParameters->bit_rate);
	}
	AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
	if (!pCodecContext) {
		logging("failed to allocated memory for AVCodecContext");
		return -1;
	}

	if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0) {
		logging("failed copy codec params to codec conetext");
		return -1;
	}

	if (avcodec_open2(pCodecContext, pCodec, NULL) < 0) {
		logging("failed to open codec through avcodec_open2");
		return -1;
	}

	AVFrame *pFrame = av_frame_alloc();
	if (!pFrame) {
		logging("failed to allocated memory for AVFrame");
		return -1;
	}

	AVPacket *pPacket = av_packet_alloc();
	if (!pPacket) {
		logging("failed to allocated memory for AVPacket");
		return -1;
	}

	int response = 0;
	int how_many_packets_to_process = 8;

	while (av_read_frame(pFormatContext, pPacket) >= 0) {
		if (pPacket->stream_index == video_stream_index) {
			logging("AVPacket->pts %" PRId64, pPacket->pts);
			response = decode_packet(pPacket, pCodecContext, pFrame);
			if (response < 0) {
				break;
			}
			if (--how_many_packets_to_process <= 0)break;
		}
		av_packet_unref(pPacket);
	}
	logging("releasing all the resources");

	avformat_close_input(&pFormatContext);
	avformat_free_context(pFormatContext);
	av_packet_free(&pPacket);
	av_frame_free(&pFrame);
	avcodec_free_context(&pCodecContext);
	return 0;
}

void logging(const char * fmt, ...)
{
	va_list args;
	fprintf(stderr, "LOG: ");
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
}

int decode_packet(AVPacket * pPacket, AVCodecContext * pCodecContext, AVFrame * pFrame)
{
	int response = avcodec_send_packet(pCodecContext, pPacket);

	if (response < 0) {
		logging("Error while sending a packet to the decoder: %s", response);
		return response;
	}

	while (response >= 0)
	{
		// Return decoded output data (into a frame) from a decoder
		// https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga11e6542c4e66d3028668788a1a74217c
		response = avcodec_receive_frame(pCodecContext, pFrame);
		if (response == AVERROR(EAGAIN)|| response == AVERROR_EOF) {
			break;
		}
		else if (response < 0) {
			logging("Error while receiving a frame from the decoder: %s", response);
			return response;
		}

		if (response >= 0) {
			logging(
				"Frame %d (type=%c, size=%d bytes) pts %d key_frame %d [DTS %d]",
				pCodecContext->frame_number,
				av_get_picture_type_char(pFrame->pict_type),
				pFrame->pkt_size,
				pFrame->pts,
				pFrame->key_frame,
				pFrame->coded_picture_number
			);

			char frame_filename[1024];
			snprintf(frame_filename, sizeof(frame_filename), "%s-%d.pgm", "frame", pCodecContext->frame_number);
			// save a grayscale frame into a .pgm file
			save_gray_frame(pFrame->data[0], pFrame->linesize[0], pFrame->width, pFrame->height, frame_filename);
		}
	}
	return 0;
}

void save_gray_frame(unsigned char * buf, int wrap, int xsize, int ysize, char * filename)
{
	FILE *f;
	int i;
	f = fopen(filename, "w");

	fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);

	for (i = 0; i < ysize; i++)
		fwrite(buf + i * wrap, 1, xsize, f);
	fclose(f);
}

/*
int main(int argc, const char *argv[]) {

	avformat_network_init();

	avdevice_register_all();
	ShowDevices *sd = new ShowDevices();
	sd->show_dshow_devices();
	sd->show_dshow_device_options();
}
*/


/* Push Stream to Server through RTMP */

static int pushRTMP(int argc, const char *argv[]) {
	AVOutputFormat *ofmt = NULL;
	AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
	AVPacket pkt;
	const char *in_filename, *out_filename;
	int ret, i;
	int videoindex = -1;
	int frame_index = 0;
	int64_t start_time = 0;
	in_filename = "Vi.flv";

	out_filename = "rtmp://192.168.11.138:1935/cctvf";
	avformat_network_init();

	// input
	if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0)
	{
		printf("Could not open input file.");
		goto end;
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		printf("Failed to retrieve input stream information");
		goto end;
	}

	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
			break;
		}
	}
	av_dump_format(ifmt_ctx, 0, in_filename, 0);

	//output
	avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", out_filename);

	if (!ofmt_ctx) {
		printf("Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt = ofmt_ctx->oformat;
	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		AVStream *in_stream = ifmt_ctx->streams[i];
		///???被否决的方法
		//AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
		//替代方案
		AVCodec *pLocalCodec = NULL;
		pLocalCodec = avcodec_find_decoder(in_stream->codecpar->codec_id);
		AVStream *out_stream = avformat_new_stream(ofmt_ctx, pLocalCodec);
		if (!out_stream) {
			printf("Failed allocating output stream\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}
		///???被否决的方法
		//ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
		//替代方案
		//参考:https://blog.csdn.net/wzz687510/article/details/81558509
		AVCodecContext *codec_ctx = avcodec_alloc_context3(pLocalCodec);
		ret = avcodec_parameters_to_context(codec_ctx, in_stream->codecpar);
		if (ret < 0) {
			printf("Failed to copy in_stream codecpar to codec context\n");
			goto end;
		}
		codec_ctx->codec_tag = 0;
		if (ofmt_ctx->oformat->flags&AVFMT_GLOBALHEADER) {
			codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}
		ret = avcodec_parameters_from_context(out_stream->codecpar, codec_ctx);
		if (ret < 0) {
			printf("Failed to copy codec context to out_stream codecpar context\n");
			goto end;
		}
	}

	av_dump_format(ofmt_ctx, 0, out_filename, 1);

	if (!(ofmt->flags&AVFMT_NOFILE)) {
		ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
		if (ret < 0) {
			printf("Could not open output URL '%s'", out_filename);
			goto end;
		}
	}
	ret = avformat_write_header(ofmt_ctx, NULL);
	if (ret < 0) {
		printf("Error occurred when opening output URL\n");
		goto end;
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
			printf("Send %8d video frames to output URL\n", frame_index);
			frame_index++;
		}
		//ret = av_write_frame(ofmt_ctx, &pkt);
		ret = av_interleaved_write_frame(ofmt_ctx, &pkt);

		if (ret < 0) {
			printf("Error muxing packet\n");
			break;
		}

		av_packet_unref(&pkt);
	}
	av_write_trailer(ofmt_ctx);
end:
	avformat_close_input(&ifmt_ctx);

	if (ofmt_ctx && !(ofmt->flags&AVFMT_NOFILE)) {
		avio_close(ofmt_ctx->pb);
	}
	avformat_free_context(ofmt_ctx);
	if (ret < 0 && ret != AVERROR_EOF) {
		printf("Error occurred.\n");
		return -1;
	}
	return 0;
}


typedef struct StreamContext {
	AVCodecContext *dec_ctx;
	AVCodecContext *enc_ctx;
} StreamContext;
static StreamContext *stream_ctx;

static AVFormatContext *ifmt_ctx;
static AVFormatContext *ofmt_ctx;

typedef struct FilteringContext {
	AVFilterContext *buffersink_ctx;
	AVFilterContext *buffersrc_ctx;
	AVFilterGraph *filter_graph;
} FilteringContext;
static FilteringContext *filter_ctx;
static int open_input_file(const char * dshow_input_name)
{
	int ret;
	unsigned int i;

	AVInputFormat *dshow_ifmt = av_find_input_format("dshow");
	//const char *dshow_input_name = "video=XiaoMi USB 2.0 Webcam";

	ifmt_ctx = NULL;
	if ((ret = avformat_open_input(&ifmt_ctx, dshow_input_name, dshow_ifmt, NULL)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
		return ret;
	}

	if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
		return ret;
	}

	stream_ctx = (StreamContext*)av_mallocz_array(ifmt_ctx->nb_streams, sizeof(*stream_ctx));
	if (!stream_ctx)
		return AVERROR(ENOMEM);

	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		AVStream *stream = ifmt_ctx->streams[i];
		AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
		AVCodecContext *codec_ctx;
		if (!dec) {
			av_log(NULL, AV_LOG_ERROR, "Failed to find decoder for stream #%u\n", i);
			return AVERROR_DECODER_NOT_FOUND;
		}
		codec_ctx = avcodec_alloc_context3(dec);
		if (!codec_ctx) {
			av_log(NULL, AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u\n", i);
			return AVERROR(ENOMEM);
		}
		ret = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context "
				"for stream #%u\n", i);
			return ret;
		}
		/* Reencode video & audio and remux subtitles etc. */
		if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
			|| codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
			if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
				codec_ctx->framerate = av_guess_frame_rate(ifmt_ctx, stream, NULL);
			/* Open decoder */
			ret = avcodec_open2(codec_ctx, dec, NULL);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
				return ret;
			}
		}
		stream_ctx[i].dec_ctx = codec_ctx;
	}

	av_dump_format(ifmt_ctx, 0, dshow_input_name, 0);
	return 0;
}

static int open_output_file(const char *filename)
{
	AVStream *out_stream;
	AVStream *in_stream;
	AVCodecContext *dec_ctx, *enc_ctx;
	AVCodec *encoder;
	int ret;
	unsigned int i;

	ofmt_ctx = NULL;
	avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", filename);
	if (!ofmt_ctx) {
		av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
		return AVERROR_UNKNOWN;
	}


	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		out_stream = avformat_new_stream(ofmt_ctx, NULL);
		if (!out_stream) {
			av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
			return AVERROR_UNKNOWN;
		}

		in_stream = ifmt_ctx->streams[i];
		dec_ctx = stream_ctx[i].dec_ctx;

		if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
			|| dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
			/* in this example, we choose transcoding to same codec */
			encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
			if (!encoder) {
				av_log(NULL, AV_LOG_FATAL, "Necessary encoder not found\n");
				return AVERROR_INVALIDDATA;
			}
			enc_ctx = avcodec_alloc_context3(encoder);
			if (!enc_ctx) {
				av_log(NULL, AV_LOG_FATAL, "Failed to allocate the encoder context\n");
				return AVERROR(ENOMEM);
			}

			/* In this example, we transcode to same properties (picture size,
			 * sample rate etc.). These properties can be changed for output
			 * streams easily using filters */
			if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
				enc_ctx->height = dec_ctx->height;
				enc_ctx->width = dec_ctx->width;
				enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
				/* take first format from list of supported formats */
				if (encoder->pix_fmts)
					enc_ctx->pix_fmt = encoder->pix_fmts[0];
				else
					enc_ctx->pix_fmt = dec_ctx->pix_fmt;
				/* video time_base can be set to whatever is handy and supported by encoder */
				enc_ctx->time_base = av_inv_q(dec_ctx->framerate);
			}
			else {
				enc_ctx->sample_rate = dec_ctx->sample_rate;
				enc_ctx->channel_layout = dec_ctx->channel_layout;
				enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);
				/* take first format from list of supported formats */
				enc_ctx->sample_fmt = encoder->sample_fmts[0];
				AVRational time_base = { 1, enc_ctx->sample_rate };
				enc_ctx->time_base = time_base;
			}

			if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
				enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

			/* Third parameter can be used to pass settings to encoder */
			ret = avcodec_open2(enc_ctx, encoder, NULL);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Cannot open video encoder for stream #%u\n", i);
				return ret;
			}
			ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Failed to copy encoder parameters to output stream #%u\n", i);
				return ret;
			}

			out_stream->time_base = enc_ctx->time_base;
			stream_ctx[i].enc_ctx = enc_ctx;
		}
		else if (dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN) {
			av_log(NULL, AV_LOG_FATAL, "Elementary stream #%d is of unknown type, cannot proceed\n", i);
			return AVERROR_INVALIDDATA;
		}
		else {
			/* if this stream must be remuxed */
			ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Copying parameters for stream #%u failed\n", i);
				return ret;
			}
			out_stream->time_base = in_stream->time_base;
		}

	}
	av_dump_format(ofmt_ctx, 0, filename, 1);

	if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
		ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Could not open output file '%s'", filename);
			return ret;
		}
	}

	/* init muxer, write output file header */
	ret = avformat_write_header(ofmt_ctx, NULL);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Error occurred when opening output file\n");
		return ret;
	}

	return 0;
}

static int init_filter(FilteringContext* fctx, AVCodecContext *dec_ctx,
	AVCodecContext *enc_ctx, const char *filter_spec)
{
	char args[512];
	int ret = 0;
	const AVFilter *buffersrc = NULL;
	const AVFilter *buffersink = NULL;
	AVFilterContext *buffersrc_ctx = NULL;
	AVFilterContext *buffersink_ctx = NULL;
	AVFilterInOut *outputs = avfilter_inout_alloc();
	AVFilterInOut *inputs = avfilter_inout_alloc();
	AVFilterGraph *filter_graph = avfilter_graph_alloc();

	if (!outputs || !inputs || !filter_graph) {
		ret = AVERROR(ENOMEM);
		goto end;
	}

	if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
		buffersrc = avfilter_get_by_name("buffer");
		buffersink = avfilter_get_by_name("buffersink");
		if (!buffersrc || !buffersink) {
			av_log(NULL, AV_LOG_ERROR, "filtering source or sink element not found\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}

		snprintf(args, sizeof(args),
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
			dec_ctx->time_base.num, dec_ctx->time_base.den,
			dec_ctx->sample_aspect_ratio.num,
			dec_ctx->sample_aspect_ratio.den);

		ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
			args, NULL, filter_graph);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
			goto end;
		}

		ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
			NULL, NULL, filter_graph);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
			goto end;
		}

		ret = av_opt_set_bin(buffersink_ctx, "pix_fmts",
			(uint8_t*)&enc_ctx->pix_fmt, sizeof(enc_ctx->pix_fmt),
			AV_OPT_SEARCH_CHILDREN);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
			goto end;
		}
	}
	else if (dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
		buffersrc = avfilter_get_by_name("abuffer");
		buffersink = avfilter_get_by_name("abuffersink");
		if (!buffersrc || !buffersink) {
			av_log(NULL, AV_LOG_ERROR, "filtering source or sink element not found\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}

		if (!dec_ctx->channel_layout)
			dec_ctx->channel_layout =
			av_get_default_channel_layout(dec_ctx->channels);
		snprintf(args, sizeof(args),
			"time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64,
			dec_ctx->time_base.num, dec_ctx->time_base.den, dec_ctx->sample_rate,
			av_get_sample_fmt_name(dec_ctx->sample_fmt),
			dec_ctx->channel_layout);
		ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
			args, NULL, filter_graph);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer source\n");
			goto end;
		}

		ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
			NULL, NULL, filter_graph);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer sink\n");
			goto end;
		}

		ret = av_opt_set_bin(buffersink_ctx, "sample_fmts",
			(uint8_t*)&enc_ctx->sample_fmt, sizeof(enc_ctx->sample_fmt),
			AV_OPT_SEARCH_CHILDREN);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot set output sample format\n");
			goto end;
		}

		ret = av_opt_set_bin(buffersink_ctx, "channel_layouts",
			(uint8_t*)&enc_ctx->channel_layout,
			sizeof(enc_ctx->channel_layout), AV_OPT_SEARCH_CHILDREN);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot set output channel layout\n");
			goto end;
		}

		ret = av_opt_set_bin(buffersink_ctx, "sample_rates",
			(uint8_t*)&enc_ctx->sample_rate, sizeof(enc_ctx->sample_rate),
			AV_OPT_SEARCH_CHILDREN);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot set output sample rate\n");
			goto end;
		}
	}
	else {
		ret = AVERROR_UNKNOWN;
		goto end;
	}

	/* Endpoints for the filter graph. */
	outputs->name = av_strdup("in");
	outputs->filter_ctx = buffersrc_ctx;
	outputs->pad_idx = 0;
	outputs->next = NULL;

	inputs->name = av_strdup("out");
	inputs->filter_ctx = buffersink_ctx;
	inputs->pad_idx = 0;
	inputs->next = NULL;

	if (!outputs->name || !inputs->name) {
		ret = AVERROR(ENOMEM);
		goto end;
	}

	if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_spec,
		&inputs, &outputs, NULL)) < 0)
		goto end;

	if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
		goto end;

	/* Fill FilteringContext */
	fctx->buffersrc_ctx = buffersrc_ctx;
	fctx->buffersink_ctx = buffersink_ctx;
	fctx->filter_graph = filter_graph;

end:
	avfilter_inout_free(&inputs);
	avfilter_inout_free(&outputs);

	return ret;
}

static int init_filters(void)
{
	const char *filter_spec;
	unsigned int i;
	int ret;
	filter_ctx = (FilteringContext*)av_malloc_array(ifmt_ctx->nb_streams, sizeof(*filter_ctx));
	if (!filter_ctx)
		return AVERROR(ENOMEM);

	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		filter_ctx[i].buffersrc_ctx = NULL;
		filter_ctx[i].buffersink_ctx = NULL;
		filter_ctx[i].filter_graph = NULL;
		if (!(ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO
			|| ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO))
			continue;


		if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			filter_spec = "null"; /* passthrough (dummy) filter for video */
		else
			filter_spec = "anull"; /* passthrough (dummy) filter for audio */
		ret = init_filter(&filter_ctx[i], stream_ctx[i].dec_ctx,
			stream_ctx[i].enc_ctx, filter_spec);
		if (ret)
			return ret;
	}
	return 0;
}

static int ex_encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt)
{
	int ret;

	/* send the frame to the encoder */
	if (frame) {
		//printf("Send frame %3"PRId64"\n", frame->pts);
		return -1;
	}

	ret = avcodec_send_frame(enc_ctx, frame);
	if (ret < 0) {
		//fprintf(stderr, "Error sending a frame for encoding\n");
		return -1;
	}

	while (ret >= 0) {
		ret = avcodec_receive_packet(enc_ctx, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return 0;
		else if (ret < 0) {
			//fprintf(stderr, "Error during encoding\n");
			return ret;
		}

		/*printf("Write packet %3"PRId64" (size=%5d)\n", pkt->pts, pkt->size);
		fwrite(pkt->data, 1, pkt->size, outfile);
		av_packet_unref(pkt);*/
	}
}

static int encode_write_frame(AVFrame *filt_frame, unsigned int stream_index, int *got_frame) {
	int ret;
	int got_frame_local;
	AVPacket enc_pkt;
	/*int(*enc_func)(AVCodecContext *, AVPacket *, const AVFrame *, int *) =
		(ifmt_ctx->streams[stream_index]->codecpar->codec_type ==
			AVMEDIA_TYPE_VIDEO) ? avcodec_encode_video2 : avcodec_encode_audio2;*/

	int(*enc_func)(AVCodecContext *, AVFrame *, AVPacket *) = ex_encode;

	if (!got_frame)
		got_frame = &got_frame_local;

	av_log(NULL, AV_LOG_INFO, "Encoding frame\n");
	/* encode filtered frame */
	enc_pkt.data = NULL;
	enc_pkt.size = 0;
	av_init_packet(&enc_pkt);
	ret = enc_func(stream_ctx[stream_index].enc_ctx, filt_frame, &enc_pkt);
	av_frame_free(&filt_frame);
	if (ret < 0)
		return ret;
	/*if (!(*got_frame))
		return 0;*/

	/* prepare packet for muxing */
	enc_pkt.stream_index = stream_index;
	av_packet_rescale_ts(&enc_pkt,
		stream_ctx[stream_index].enc_ctx->time_base,
		ofmt_ctx->streams[stream_index]->time_base);

	av_log(NULL, AV_LOG_DEBUG, "Muxing frame\n");
	/* mux encoded frame */
	ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);

	//todo
	av_packet_unref(&enc_pkt);
	return ret;
}

static int filter_encode_write_frame(AVFrame *frame, unsigned int stream_index) {
	int ret;
	AVFrame *filt_frame;

	av_log(NULL, AV_LOG_INFO, "Pushing decoded frame to filters\n");
	/* push the decoded frame into the filtergraph */
	/*ret = av_buffersrc_add_frame_flags(filter_ctx[stream_index].buffersrc_ctx,
		frame, 0);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
		return ret;
	}*/

	/* pull filtered frames from the filtergraph */
	while (1) {
		filt_frame = av_frame_alloc();
		if (!filt_frame) {
			ret = AVERROR(ENOMEM);
			break;
		}
		av_log(NULL, AV_LOG_INFO, "Pulling filtered frame from filters\n");
		//ret = av_buffersink_get_frame(filter_ctx[stream_index].buffersink_ctx,
		//	filt_frame);
		//if (ret < 0) {
		//	/* if no more frames for output - returns AVERROR(EAGAIN)
		//	 * if flushed and no more frames for output - returns AVERROR_EOF
		//	 * rewrite retcode to 0 to show it as normal procedure completion
		//	 */
		//	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
		//		ret = 0;
		//	av_frame_free(&filt_frame);
		//	break;
		//}

		filt_frame->pict_type = AV_PICTURE_TYPE_NONE;
		ret = encode_write_frame(filt_frame, stream_index, NULL);
		if (ret < 0)
			break;
	}

	return ret;
}

int ex_decode_packet(AVPacket * pPacket, AVCodecContext * pCodecContext, AVFrame * pFrame)
{
	int response = avcodec_send_packet(pCodecContext, pPacket);

	if (response < 0) {
		logging("Error while sending a packet to the decoder: %s", response);
		return response;
	}

	while (response >= 0)
	{
		// Return decoded output data (into a frame) from a decoder
		response = avcodec_receive_frame(pCodecContext, pFrame);
		if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
			break;
		}
		else if (response < 0) {
			logging("Error while receiving a frame from the decoder: %s", response);
			return response;
		}

		if (response >= 0) {
			logging(
				"Frame %d (type=%c, size=%d bytes) pts %d key_frame %d [DTS %d]",
				pCodecContext->frame_number,
				av_get_picture_type_char(pFrame->pict_type),
				pFrame->pkt_size,
				pFrame->pts,
				pFrame->key_frame,
				pFrame->coded_picture_number
			);
		}
	}
	return 0;
}


static int flush_encoder(unsigned int stream_index)
{
	int ret;
	int got_frame;

	if (!(stream_ctx[stream_index].enc_ctx->codec->capabilities &
		AV_CODEC_CAP_DELAY))
		return 0;

	while (1) {
		av_log(NULL, AV_LOG_INFO, "Flushing stream #%u encoder\n", stream_index);
		ret = encode_write_frame(NULL, stream_index, &got_frame);
		if (ret < 0)
			break;
		/*if (!got_frame)
			return 0;*/
	}
	return ret;
}

static int pushRTMPflv(int argc, const char *argv[]) {

	int ret;
	AVPacket packet;
	AVFrame *frame = NULL;
	enum AVMediaType type;
	unsigned int stream_index;
	unsigned int i;
	int got_frame;
	int(*dec_func)(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame);

	const char *in_filename = "Vi.flv";

	const char *out_filename = "rtmp://192.168.11.138:1935/cctvf";
	avformat_network_init();
	avdevice_register_all();

	const char *dshow_input_name = "video=XiaoMi USB 2.0 Webcam";

	open_input_file(dshow_input_name);

	open_output_file(out_filename);

	init_filters();

	/* read all packets */
	while (1) {
		if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0)
			break;
		stream_index = packet.stream_index;
		type = ifmt_ctx->streams[packet.stream_index]->codecpar->codec_type;
		av_log(NULL, AV_LOG_DEBUG, "Demuxer gave frame of stream_index %u\n",
			stream_index);

		if (filter_ctx[stream_index].filter_graph) {
			av_log(NULL, AV_LOG_DEBUG, "Going to reencode&filter the frame\n");
			frame = av_frame_alloc();
			if (!frame) {
				ret = AVERROR(ENOMEM);
				break;
			}
			av_packet_rescale_ts(&packet,
				ifmt_ctx->streams[stream_index]->time_base,
				stream_ctx[stream_index].dec_ctx->time_base);
			//dec_func = (type == AVMEDIA_TYPE_VIDEO) ? avcodec_decode_video2 : avcodec_decode_audio4;
			//both video and avdio
			dec_func = ex_decode_packet;
			/*ret = dec_func(stream_ctx[stream_index].dec_ctx, frame,
				&got_frame, &packet);*/
			ret = dec_func(&packet, stream_ctx[stream_index].dec_ctx, frame);
			if (ret < 0) {
				av_frame_free(&frame);
				av_log(NULL, AV_LOG_ERROR, "Decoding failed\n");
				break;
			}

			//if (got_frame) {
				frame->pts = frame->best_effort_timestamp;
				ret = filter_encode_write_frame(frame, stream_index);
				av_frame_free(&frame);
				if (ret < 0)
					goto end;
			//}
			//else {
			//	av_frame_free(&frame);
			//}
		}
		else {
			/* remux this frame without reencoding */
			av_packet_rescale_ts(&packet,
				ifmt_ctx->streams[stream_index]->time_base,
				ofmt_ctx->streams[stream_index]->time_base);

			ret = av_interleaved_write_frame(ofmt_ctx, &packet);
			if (ret < 0)
				goto end;
		}
		av_packet_unref(&packet);
	}

	/* flush filters and encoders */
	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		/* flush filter */
		if (!filter_ctx[i].filter_graph)
			continue;
		ret = filter_encode_write_frame(NULL, i);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Flushing filter failed\n");
			goto end;
		}

		/* flush encoder */
		ret = flush_encoder(i);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Flushing encoder failed\n");
			goto end;
		}
	}

	av_write_trailer(ofmt_ctx);

end:
	av_packet_unref(&packet);
	av_frame_free(&frame);
	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		avcodec_free_context(&stream_ctx[i].dec_ctx);
		if (ofmt_ctx && ofmt_ctx->nb_streams > i && ofmt_ctx->streams[i] && stream_ctx[i].enc_ctx)
			avcodec_free_context(&stream_ctx[i].enc_ctx);
		if (filter_ctx && filter_ctx[i].filter_graph)
			avfilter_graph_free(&filter_ctx[i].filter_graph);
	}
	av_free(filter_ctx);
	av_free(stream_ctx);
	avformat_close_input(&ifmt_ctx);
	if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
		avio_closep(&ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);

	if (ret < 0)
		av_log(NULL, AV_LOG_ERROR, "Error occurred:\n");

	return ret ? 1 : 0;
}

int main(int argc, const char *argv[]) {
	pushRTMPflv(argc, argv);
}

/*
int main(int argc, const char *argv[]) {
	avformat_network_init();

	avdevice_register_all();
	AVInputFormat *av = find_input_format("dshow");
	if (!av) {
		printf("could find dshow devices");
	}
}
*/

/*
int main(int argc, const char *argv[]) {
	AVOutputFormat *ofmt = NULL;
	AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
	AVPacket pkt;
	const char *in_filename, *out_filename;
	int ret, i;
	int videoindex = -1;
	int frame_index = 0;
	int64_t start_time = 0;
	in_filename = "movie.flv";


	avformat_network_init();

	// input
	if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
		printf("Could not open input file.");
		return -1;
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		printf("Failed to retrieve input stream information");
		return -1;
	}

	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
			break;
		}
	}
	av_dump_format(ifmt_ctx, 0, in_filename, 0);
}
*/

/*
int main(int argc, const char *argv[]) {
	avformat_network_init();
	avdevice_register_all();

	logging("initializing all the containers,codecs and protocols");

	const char *input_filename = "Vi.flv";

	AVFormatContext *pFormatContext = avformat_alloc_context();
	if (!pFormatContext) {
		logging("ERROR could not allocate memory for Format Context");
		return -1;
	}

	logging("opening the input file (%s) and loading format (container) header", input_filename);

	if (avformat_open_input(&pFormatContext, input_filename, NULL, NULL) != 0) {
		logging("ERROR could not open the file");
		return -1;
	}
	int ret = 0;
	ret = avformat_find_stream_info(pFormatContext, NULL);
	if (ret < 0) {
		fprintf(stderr, "Could not find stream information\n");
		return -1;
	}
	av_dump_format(pFormatContext, 0, input_filename, 0);
	//AvioReading *ar = new AvioReading();

	//ar->run(argc, argv);
	return 0;
}
*/