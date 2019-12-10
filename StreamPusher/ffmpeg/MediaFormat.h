#pragma once
#ifndef _MEDIA_FORMAT_H_INCLUDED_
#define _MEDIA_FORMAT_H_INCLUDED_

#include "ffmpegHeader.h"
#include "noncopyable.h"
class InputInformation;
using namespace std;

typedef std::weak_ptr<sp_log_t> wpLog;
typedef std::shared_ptr<sp_log_t> spLog;

enum IO_Type{
	IO_TYPE_INPUT,
	IO_TYPE_OUTPUT
};

class MediaFormat :noncopyable {
public:
	MediaFormat(const string &name,
		IO_Type type = IO_TYPE_INPUT);
	~MediaFormat();
	virtual int Open() = 0;
	//��ȡ��ʽ��������Ϣ
	InputInformation *getInformation() const;
	void setLogger(std::weak_ptr<sp_log_t> logger);
private:
	typedef struct StreamContext {
		AVCodecContext *ctx;
	} StreamContext;
	enum {
		EXTEN_CAMERA,
		EXTEN_FILE
	}InputType;
	string _name;
	IO_Type _type;
	std::shared_ptr<InputInformation> _info;
	AVFormatContext *fmt_ctx;
	StreamContext *stream_ctx;
	//�����������ͺ͸�ʽ
	int parseFormat();
	//������������ʽ
	void parseStreamFormat(int i, int is_output);
	//���������ļ�����չ��
	//����û�ж�ȡ�ļ���ͷ��Ϣ,��������������������жϻ��������ַ�������չ��.
	//�������չ��,�鿴ffmpeg���Ƿ�֧�ָø�ʽ,
	//û����չ��Ĭ�ϱ�ʾ����ͷ����
	int checkExtensions();
	void print_fps(double d, const char *postfix);
	void writeLog(const char *fmt, ...);
	wpLog _logger;
};

#endif // !_MEDIA_FORMAT_H_INCLUDED_
