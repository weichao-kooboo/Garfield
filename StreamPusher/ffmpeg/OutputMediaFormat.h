#pragma once
#ifndef _OUTPUT_MEDIA_FORMAT_H_INCLUDED_
#define	_OUTPUT_MEDIA_FORMAT_H_INCLUDED_
#include "MediaFormat.h"
class OutputMediaFormat :public MediaFormat {
public:
	OutputMediaFormat(const string &name, const weak_ptr<MediaFormat> &input_media_format);
	~OutputMediaFormat();
	int Open() override;
private:
	weak_ptr<MediaFormat> _input_media_format;
};

#endif // !_OUTPUT_MEDIA_FORMAT_H_INCLUDED_
