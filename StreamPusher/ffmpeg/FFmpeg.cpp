#include "FFmpeg.h"
#include "MediaFilter.h"
#include "OutputMediaFormat.h"
#include "InputMediaFormat.h"
#include "Logger.h"

FFmpeg::FFmpeg(const shared_ptr<MediaFilter>& mediaFilter, const weak_ptr<Logger>& logger)
	:_mediaFilter(mediaFilter),
	_logger(logger)
{
	codeCallBack cb = std::bind(&FFmpeg::decodeCallBack, this, _1, _2, _3, _4);
}

FFmpeg::~FFmpeg()
{
	_mediaFilter.reset();
}

int FFmpeg::start()
{
	shared_ptr<FilteringContext> filter_ctx = _mediaFilter->getFilteringCtx().lock();
	if (!filter_ctx) {
		writeLog("the filter ctx pointer have been release");
		return AVERROR(ENOMEM);
	}
	shared_ptr<OutputMediaFormat> output_format = _mediaFilter->getOutputMediaFormat().lock();
	if (!output_format) {
		writeLog("the output format pointer have been release");
		return AVERROR(ENOMEM);
	}
	shared_ptr<InputMediaFormat> input_format = output_format->getInputMediaFormat().lock();
	if (!input_format) {
		writeLog("the input format pointer have been release");
		return AVERROR(ENOMEM);
	}

}


void FFmpeg::readFrameLoop(shared_ptr<FilteringContext>& _filter_ctx
	, shared_ptr<OutputMediaFormat>& _output_format
	, shared_ptr<InputMediaFormat>& _input_format)
{
	int ret;
	AVPacket packet;
	AVFrame *frame = NULL;
	AVFormatContext *ifmt_ctx;
	unsigned int stream_index;
	enum AVMediaType type;
	FilteringContext *filter_ctx;
	StreamContext *stream_ctx;

	ifmt_ctx = _input_format->getFormatContext();
	filter_ctx = &(*_filter_ctx);
	stream_ctx = _input_format->getStreamContext();
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
				stream_ctx[stream_index].ctx->time_base);
			ret = decodeCallBack(&packet, stream_ctx[stream_index].ctx, frame, stream_index);
			if (ret < 0) {
				av_frame_free(&frame);
				writeLog("Decoding failed\n");
				break;
			}
		}
	}
}

int FFmpeg::encodeCallBack(AVPacket * pPacket, AVCodecContext * pCodecContext, AVFrame * pFrame, unsigned int stream_index)
{
	return 0;
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
			ret = filterEncodeWriteFrame(pFrame, stream_index);
			//av_frame_free(&frame);
			if (ret < 0) {
				return ret;
			}

		}
	}
	return 0;
}

int FFmpeg::filterEncodeWriteFrame(AVFrame * frame, unsigned int stream_index)
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
		ret = encode_write_frame(filt_frame, stream_index);
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
