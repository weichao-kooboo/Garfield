#pragma once
#ifndef _SP_LOGGER_H_INCLUDED_
#define _SP_LOGGER_H_INCLUDED_

#include "ffmpegHeader.h"
#include "noncopyable.h"

typedef std::weak_ptr<sp_log_t> wpLog;
typedef std::shared_ptr<sp_log_t> spLog;

class Logger :noncopyable {
public:
	Logger(const std::weak_ptr<sp_log_t> &logger);
	~Logger();
	void writeLog(const char *fmt, ...);
	void setLogger(const std::weak_ptr<sp_log_t> &logger);
private:
	wpLog _logger;
};

#endif // !_SP_LOGGER_H_INCLUDED_
