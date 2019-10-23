#pragma once
class FFmpegMethods
{
public:
	FFmpegMethods();
	~FFmpegMethods();

	static void aVDictionary();

	static int av_input_video_device_next_test();
private:
	static int show_sinks_sources_parse_arg(const char *arg, char **dev, AVDictionary **opts);

	static int print_device_sources(AVInputFormat *ifmt, AVDictionary *opts);
};

