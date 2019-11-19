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
	if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
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

int main(int argc, const char *argv[]) {
	avformat_network_init();
	avdevice_register_all();
	AvioReading *ar = new AvioReading();
	ar->run(argc, argv);
	return 0;
}