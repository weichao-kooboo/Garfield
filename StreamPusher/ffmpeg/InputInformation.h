#pragma once
#ifndef _INPUT_INFORMATION_H_INCLUDED_
#define	_INPUT_INFORMATION_H_INCLUDED_

#include <string>

class InputInformation {
public:
	InputInformation();
	~InputInformation();
private:
	typedef struct {
		int index;
	}Stream;
	char *_format;
	char *_url;
	char *_duration;
	char *_start;
	char *_bitrate;
	Stream *_stream;
};

#endif // !_INPUT_INFORMATION_H_INCLUDED_
