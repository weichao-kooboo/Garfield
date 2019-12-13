#include "Logger.h"

Logger::Logger(const std::weak_ptr<sp_log_t>& logger):
	_logger(logger)
{
}

Logger::~Logger()
{
	_logger.reset();
}

void Logger::writeLog(const char * fmt, va_list args)
{
	//todo lock
	spLog local_logs = _logger.lock();
	if (!local_logs) {
		sp_log_stderr(0, "logger pointer have been release");
	}
	sp_log_t* origin_ptr = &(*local_logs);
	//va_list  args;
	if (origin_ptr->log_level >= SP_LOG_ALERT) {
		//va_start(args, fmt);
		sp_log_error_msg(SP_LOG_ALERT, origin_ptr, 0, fmt, args);
		//va_end(args);
	}
}

void Logger::setLogger(const std::weak_ptr<sp_log_t>& logger)
{
	std::weak_ptr<sp_log_t> local_log;
	{
		//todo:lock log obj
		_logger.swap(local_log);
		_logger = logger;
	}
	local_log.reset();
}
