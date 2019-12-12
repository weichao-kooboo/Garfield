#pragma once
#ifndef _OUTPUT_MEDIA_FORMAT_H_INCLUDED_
#define	_OUTPUT_MEDIA_FORMAT_H_INCLUDED_
#include "InputMediaFormat.h"
class OutputMediaFormat :public MediaFormat {
public:
	OutputMediaFormat(const string &name,
		const weak_ptr<Logger> &logger,
		const shared_ptr<InputMediaFormat> &input_media_format);
	~OutputMediaFormat();
	int Open() override;
	weak_ptr<InputMediaFormat> getInputMediaFormat();
private:
	shared_ptr<InputMediaFormat> _input_media_format;
};

#endif // !_OUTPUT_MEDIA_FORMAT_H_INCLUDED_
