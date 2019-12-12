#include "FFmpeg.h"
#include "Logger.h"

FFmpeg::FFmpeg(const weak_ptr<Logger>& logger):
	_logger(logger)
{
}

FFmpeg::~FFmpeg()
{
}

int FFmpeg::start(InputMediaFormat & input_media_format, OutputMediaFormat & output_media_format, MediaFilter & _filter_ctx)
{
	ifmt_ctx = input_media_format.getFormatContext();
	istream_ctx = input_media_format.getStreamContext();
	ofmt_ctx = output_media_format.getFormatContext();
	ostream_ctx = output_media_format.getStreamContext();
	filter_ctx = _filter_ctx.getFilteringCtx();

	return 0;
}


int FFmpeg::readFrameLoop()
{
	int ret;
	AVPacket packet;
	AVFrame *frame = NULL;
	unsigned int i;
	unsigned int stream_index;
	enum AVMediaType type;

	while (1) {
		if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0)
			break;
		stream_index = packet.stream_index;
		type = ifmt_ctx->streams[packet.stream_index]->codecpar->codec_type;
		writeLog("Demuxer gave frame of stream_index %u\n",
			stream_index);

		if (filter_ctx[stream_index].filter_graph) {
			writeLog("Going to reencode&filter the frame\n");
			frame = av_frame_alloc();
			if (!frame) {
				ret = AVERROR(ENOMEM);
				break;
			}
			av_packet_rescale_ts(&packet,
				ifmt_ctx->streams[stream_index]->time_base,
				istream_ctx[stream_index].ctx->time_base);
			ret = decodeCallBack(&packet, istream_ctx[stream_index].ctx, frame, stream_index);
			if (ret < 0) {
				av_frame_free(&frame);
				writeLog("Decoding failed\n");
				break;
			}
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
		ret = filterDecodeReadFrame(NULL, i);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Flushing filter failed\n");
			goto end;
		}

		/* flush encoder */
		ret = flushEncoder(i);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Flushing encoder failed\n");
			goto end;
		}
	}

	av_write_trailer(ofmt_ctx);

end:
	av_packet_unref(&packet);
	av_frame_free(&frame);

	if (ret < 0)
		av_log(NULL, AV_LOG_ERROR, "Error occurred:\n");

	return ret ? 1 : 0;
}

int FFmpeg::encodeCallBack(AVCodecContext *pCodecContext, AVFrame *pFrame, AVPacket *pPacket, unsigned int stream_index)
{
	int ret;

	/* send the frame to the encoder */
	if (pFrame) {
		printf("pts %d\n", pFrame->pts);
	}

	ret = avcodec_send_frame(pCodecContext, pFrame);
	if (ret < 0) {
		//fprintf(stderr, "Error sending a frame for encoding\n");
		return -1;
	}

	while (ret >= 0) {
		ret = avcodec_receive_packet(pCodecContext, pPacket);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return 0;
		else if (ret < 0) {
			//fprintf(stderr, "Error during encoding\n");
			return ret;
		}

		/* prepare packet for muxing */
		pPacket->stream_index = stream_index;
		av_packet_rescale_ts(pPacket,
			ostream_ctx[stream_index].ctx->time_base,
			ofmt_ctx->streams[stream_index]->time_base);

		av_log(NULL, AV_LOG_DEBUG, "Muxing frame\n");
		/* mux encoded frame */
		ret = av_interleaved_write_frame(ofmt_ctx, pPacket);
	}
}

int FFmpeg::decodeCallBack(AVPacket * pPacket, AVCodecContext * pCodecContext, AVFrame * pFrame, unsigned int stream_index)
{
	int response = avcodec_send_packet(pCodecContext, pPacket);
	int ret = 0;

	if (response < 0) {
		writeLog("Error while sending a packet to the decoder: %s", response);
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
			writeLog("Error while receiving a frame from the decoder: %s", response);
			return response;
		}

		if (response >= 0) {
			writeLog(
				"Frame %d (type=%c, size=%d bytes) pts %d key_frame %d [DTS %d]",
				pCodecContext->frame_number,
				av_get_picture_type_char(pFrame->pict_type),
				pFrame->pkt_size,
				pFrame->pts,
				pFrame->key_frame,
				pFrame->coded_picture_number
			);


			pFrame->pts = pFrame->best_effort_timestamp;
			ret = filterDecodeReadFrame(pFrame, stream_index);
			//av_frame_free(&frame);
			if (ret < 0) {
				return ret;
			}

		}
	}
	return 0;
}

int FFmpeg::filterDecodeReadFrame(AVFrame * frame, unsigned int stream_index)
{
	int ret;
	AVFrame *filt_frame;

	av_log(NULL, AV_LOG_INFO, "Pushing decoded frame to filters\n");
	/* push the decoded frame into the filtergraph */
	ret = av_buffersrc_add_frame_flags(filter_ctx[stream_index].buffersrc_ctx,
		frame, 0);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
		return ret;
	}

	/* pull filtered frames from the filtergraph */
	while (1) {
		filt_frame = av_frame_alloc();
		if (!filt_frame) {
			ret = AVERROR(ENOMEM);
			break;
		}
		av_log(NULL, AV_LOG_INFO, "Pulling filtered frame from filters\n");
		ret = av_buffersink_get_frame(filter_ctx[stream_index].buffersink_ctx,
			filt_frame);
		if (ret < 0) {
			/* if no more frames for output - returns AVERROR(EAGAIN)
			 * if flushed and no more frames for output - returns AVERROR_EOF
			 * rewrite retcode to 0 to show it as normal procedure completion
			 */
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				ret = 0;
			av_frame_free(&filt_frame);
			break;
		}

		filt_frame->pict_type = AV_PICTURE_TYPE_NONE;
		ret = encodeWriteFrame(filt_frame, stream_index);
		if (ret < 0)
			break;
	}

	return ret;
}

int FFmpeg::encodeWriteFrame(AVFrame * filt_frame, unsigned int stream_index)
{
	int ret;
	AVPacket enc_pkt;

	av_log(NULL, AV_LOG_INFO, "Encoding frame\n");
	/* encode filtered frame */
	enc_pkt.data = NULL;
	enc_pkt.size = 0;
	av_init_packet(&enc_pkt);
	ret = encodeCallBack(ostream_ctx[stream_index].ctx, filt_frame, &enc_pkt, stream_index);
	av_frame_free(&filt_frame);
	if (ret < 0)
		return ret;

	//todo
	av_packet_unref(&enc_pkt);
	return ret;
}

int FFmpeg::flushEncoder(unsigned int stream_index)
{
	int ret;

	if (!(ostream_ctx[stream_index].ctx->codec->capabilities &
		AV_CODEC_CAP_DELAY))
		return 0;

	while (1) {
		av_log(NULL, AV_LOG_INFO, "Flushing stream #%u encoder\n", stream_index);
		ret = encodeWriteFrame(NULL, stream_index);
		if (ret < 0)
			break;
	}
	return ret;
}

void FFmpeg::writeLog(const char * fmt, ...)
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
