#include "RtmpPusher.h"

RtmpPusher::RtmpPusher()
{
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

void RtmpPusher::writeLog(const char *fmt, ...)
{
	//todo lock
	spLog local_logs = _logger.lock();
	if (!local_logs) {
		sp_log_stderr(0, "logger pointer have been release");
	}
	sp_log_t* origin_ptr = &(*local_logs);
	
	sp_log_error(SP_LOG_ALERT, origin_ptr, 0,
		fmt, ...);
}
