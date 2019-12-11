#pragma once
#ifndef _INPUT_MEDIA_FORMAT_H_INCLUDED_
#define	_INPUT_MEDIA_FORMAT_H_INCLUDED_
#include "MediaFormat.h"
class InputMediaFormat :public MediaFormat {
public:
	InputMediaFormat(const string &name);
	~InputMediaFormat();
	int Open() override;
};

#endif // !_INPUT_MEDIA_FORMAT_H_INCLUDED_
