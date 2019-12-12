#include "OutputMediaFormat.h"

OutputMediaFormat::OutputMediaFormat(const string &name, 
	const weak_ptr<Logger> &logger,
	const shared_ptr<InputMediaFormat> &input_media_format)
	:MediaFormat(name, logger, IO_TYPE_OUTPUT),
	_input_media_format(input_media_format)
{

}

OutputMediaFormat::~OutputMediaFormat()
{
	_input_media_format.reset();
}

int OutputMediaFormat::Open()
{
	AVStream *out_stream;
	AVStream *in_stream;
	AVCodecContext *dec_ctx, *enc_ctx;
	AVCodec *encoder;
	int ret;
	unsigned int i;
	AVFormatContext *ifmt_ctx;
	StreamContext *istream_ctx;

	fmt_ctx = NULL;
	const char *fmt_name = NULL;

	if (InputType == EXTEN_CAMERA) {
		fmt_name = "flv";
	}
	avformat_alloc_output_context2(&fmt_ctx, NULL, fmt_name, _name.c_str());
	if (!fmt_ctx) {
		writeLog("Could not create output context\n");
		return AVERROR_UNKNOWN;
	}

	if (_input_media_format->getType() != IO_TYPE_INPUT) {
		writeLog("input media format isn't input format");
		return AVERROR_UNKNOWN;
	}
	ifmt_ctx = _input_media_format->getFormatContext();
	istream_ctx = _input_media_format->getStreamContext();

	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		out_stream = avformat_new_stream(fmt_ctx, NULL);
		if (!out_stream) {
			writeLog("Failed allocating output stream\n");
			return AVERROR_UNKNOWN;
		}

		in_stream = fmt_ctx->streams[i];
		dec_ctx = istream_ctx[i].ctx;

		if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
			|| dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
			/* in this example, we choose transcoding to same codec */
			encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
			if (!encoder) {
				writeLog("Necessary encoder not found\n");
				return AVERROR_INVALIDDATA;
			}
			enc_ctx = avcodec_alloc_context3(encoder);
			if (!enc_ctx) {
				writeLog("Failed to allocate the encoder context\n");
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

			if (fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
				enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

			/* Third parameter can be used to pass settings to encoder */
			ret = avcodec_open2(enc_ctx, encoder, NULL);
			if (ret < 0) {
				writeLog("Cannot open video encoder for stream #%u\n", i);
				return ret;
			}
			ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
			if (ret < 0) {
				writeLog("Failed to copy encoder parameters to output stream #%u\n", i);
				return ret;
			}

			out_stream->time_base = enc_ctx->time_base;
			stream_ctx[i].ctx = enc_ctx;
		}
		else if (dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN) {
			writeLog("Elementary stream #%d is of unknown type, cannot proceed\n", i);
			return AVERROR_INVALIDDATA;
		}
		else {
			/* if this stream must be remuxed */
			ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
			if (ret < 0) {
				writeLog("Copying parameters for stream #%u failed\n", i);
				return ret;
			}
			out_stream->time_base = in_stream->time_base;
		}
	}

	parseFormat();

	if (!(fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
		ret = avio_open(&fmt_ctx->pb, _name.c_str(), AVIO_FLAG_WRITE);
		if (ret < 0) {
			writeLog("Could not open output file '%s'", _name);
			return ret;
		}
	}

	/* init muxer, write output file header */
	ret = avformat_write_header(fmt_ctx, NULL);
	if (ret < 0) {
		writeLog("Error occurred when opening output file\n");
		return ret;
	}

	return 0;
}

weak_ptr<InputMediaFormat> OutputMediaFormat::getInputMediaFormat()
{
	return weak_ptr<InputMediaFormat>(_input_media_format);
}
