#include "RtmpPusher.h"

RtmpPusher::RtmpPusher(sp_log_t *logger)
{
	_logger = logger;
}

RtmpPusher::~RtmpPusher()
{

}

int RtmpPusher::push(const char * input_name, const char * output_name)
{
	if (!input_name || !output_name)
		return -1;
	openInput();
}

int RtmpPusher::openInput()
{
	int ret = 0;
	sp_log_error(SP_LOG_ALERT, _logger, 0,
		" open %s input failed", _input_name);
	if ((ret = avformat_open_input(&_ifmt_ctx, _input_name, 0, 0)) < 0) {
		sp_log_error(SP_LOG_ALERT, _logger, 0,
			" open %s input failed", _input_name);
		return -1;
	}
	return 0;
}

void RtmpPusher::openOutput()
{
}

void RtmpPusher::pushStream()
{
}
