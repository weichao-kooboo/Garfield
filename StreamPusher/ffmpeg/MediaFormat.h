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
	//获取格式的所有信息
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
	//解析输入类型和格式
	int parseFormat();
	//解析输入流格式
	void parseStreamFormat(int i, int is_output);
	//解析输入文件的扩展名
	//由于没有读取文件的头信息,所以在这里的输入类型判断基于输入字符串的扩展名.
	//如果有扩展名,查看ffmpeg中是否支持该格式,
	//没有扩展名默认表示摄像头名称
	int checkExtensions();
	void print_fps(double d, const char *postfix);
	void writeLog(const char *fmt, ...);
	wpLog _logger;
};

#endif // !_MEDIA_FORMAT_H_INCLUDED_
