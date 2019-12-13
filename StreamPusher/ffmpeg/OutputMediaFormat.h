#pragma once
#ifndef _OUTPUT_MEDIA_FORMAT_H_INCLUDED_
#define	_OUTPUT_MEDIA_FORMAT_H_INCLUDED_
#include "InputMediaFormat.h"
class OutputMediaFormat :public MediaFormat {
public:
	OutputMediaFormat(const string &name,
		const weak_ptr<Logger> &logger);
	~OutputMediaFormat();
	int Open() override;
	int Open(InputMediaFormat &_input_media_format);
};

#endif // !_OUTPUT_MEDIA_FORMAT_H_INCLUDED_
