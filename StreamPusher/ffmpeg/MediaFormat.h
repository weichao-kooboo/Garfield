#pragma once
#ifndef _MEDIA_FORMAT_H_INCLUDED_
#define _MEDIA_FORMAT_H_INCLUDED_

#include "ffmpegHeader.h"
#include "noncopyable.h"
class InputInformation;
class Logger;
using namespace std;

enum IO_Type{
	IO_TYPE_INPUT,
	IO_TYPE_OUTPUT
};
typedef struct StreamContext {
	AVCodecContext *ctx;
} StreamContext;

class MediaFormat :noncopyable {
public:
	MediaFormat(const string &name,
		const weak_ptr<Logger> &logger,
		IO_Type type = IO_TYPE_INPUT);
	~MediaFormat();
	virtual int Open() = 0;
	//��ȡ��ʽ��������Ϣ
	InputInformation *getInformation() const;
	AVFormatContext *getFormatContext() const;
	StreamContext *getStreamContext() const;
	IO_Type getType() const;
protected:
	enum {
		EXTEN_CAMERA,
		EXTEN_FILE
	}InputType;
	string _name;
	//���������ļ�����չ��
	//����û�ж�ȡ�ļ���ͷ��Ϣ,��������������������жϻ��������ַ�������չ��.
	//�������չ��,�鿴ffmpeg���Ƿ�֧�ָø�ʽ,
	//û����չ��Ĭ�ϱ�ʾ����ͷ����
	int checkExtensions();
	std::shared_ptr<InputInformation> _info;
	AVFormatContext *fmt_ctx;
	StreamContext *stream_ctx;
	//�����������ͺ͸�ʽ
	int parseFormat();
	void writeLog(const char * fmt, ...);
private:
	IO_Type _type;
	//������������ʽ
	void parseStreamFormat(int i, int is_output);
	void print_fps(double d, const char *postfix);
    weak_ptr<Logger> _logger;
};

#endif // !_MEDIA_FORMAT_H_INCLUDED_
