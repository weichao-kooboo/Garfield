#pragma once
#ifndef _MEDIA_FORMAT_H_INCLUDED_
#define _MEDIA_FORMAT_H_INCLUDED_

#include "ffmpegHeader.h"
class InputInformation;
using namespace std;

typedef std::weak_ptr<sp_log_t> wpLog;
typedef std::shared_ptr<sp_log_t> spLog;

class MediaFormat {
public:
	MediaFormat(const string &name);
	~MediaFormat();
	int Open();
	//��ȡ��ʽ��������Ϣ
	InputInformation *getInformation();
	void setLogger(std::weak_ptr<sp_log_t> logger);
private:
	typedef struct StreamContext {
		AVCodecContext *ctx;
	} StreamContext;
    string _name;
	InputInformation *_info;
	AVFormatContext *fmt_ctx;
	StreamContext *stream_ctx;
	//�����������ͺ͸�ʽ
	int parseFormat();
	//���������ļ�����չ��
	//����û�ж�ȡ�ļ���ͷ��Ϣ,��������������������жϻ��������ַ�������չ��.
	//�������չ��,�鿴ffmpeg���Ƿ�֧�ָø�ʽ,
	//û����չ��Ĭ�ϱ�ʾ����ͷ����
	int checkExtensions();
	void writeLog(const char *fmt, ...);
	wpLog _logger;
};

#endif // !_MEDIA_FORMAT_H_INCLUDED_
