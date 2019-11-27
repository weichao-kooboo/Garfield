#pragma once
#ifndef _SP_LOGGER_H_INCLUDED_
#define _SP_LOGGER_H_INCLUDED_

#include "ffmpegHeader.h"
using namespace std;

class Logger {
public:
	typedef std::shared_ptr<Logger> Ptr;
	~Logger() {

	}
	Logger(Logger&) = delete;
	Logger& operator=(const Logger&) = delete;
	static Ptr get_instance() {

	}
private:
	Logger() {

	}
	static Ptr m_instance_ptr;
};

#endif // !_SP_LOGGER_H_INCLUDED_
