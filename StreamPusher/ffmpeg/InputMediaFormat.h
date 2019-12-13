#pragma once
#ifndef _INPUT_MEDIA_FORMAT_H_INCLUDED_
#define	_INPUT_MEDIA_FORMAT_H_INCLUDED_
#include "MediaFormat.h"
class InputMediaFormat :public MediaFormat {
public:
	InputMediaFormat(const string &name, const weak_ptr<Logger> &logger);
	~InputMediaFormat();
	int Open() override;
};

#endif // !_INPUT_MEDIA_FORMAT_H_INCLUDED_
