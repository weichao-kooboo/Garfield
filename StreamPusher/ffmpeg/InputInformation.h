#pragma once
#ifndef _INPUT_INFORMATION_H_INCLUDED_
#define	_INPUT_INFORMATION_H_INCLUDED_

typedef struct {
	char *WMFSKDNeeded;
	char *DeviceConformanceTemplate;
	char *WMFSKDVersion;
	char *IsVBR;
	char *VBRPeak;
	char *BufferAverage;
	char *Encoder;
}MetaData;

typedef struct {
	int index;
}Stream;

class InputInformation {
public:
	InputInformation();
	~InputInformation();
private:
	char *_format;
	char *_url;
	char *_duration;
	char *_start;
	char *_bitrate;
	MetaData *_md;
	Stream *_stream;
};

#endif // !_INPUT_INFORMATION_H_INCLUDED_
