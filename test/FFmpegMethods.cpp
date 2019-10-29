#include "pch.h"
#include "FFmpegMethods.h"


FFmpegMethods::FFmpegMethods()
{
}


FFmpegMethods::~FFmpegMethods()
{
}

void FFmpegMethods::aVDictionary()
{
	// AVDictionary Test
	AVDictionary *in_opts = NULL;
	avformat_network_init();
	int argc = 4;
	char* argv[4];

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-oi") && i + 1 < argc) {
			if (av_dict_parse_string(&in_opts, argv[i + 1], "=", ":", 0) < 0) {
				fprintf(stderr, "Cannot parse option string %s\n", argv[i + 1]);
			}
			i++;
		}
	}
	AVDictionaryEntry *ade = NULL;
	ade = av_dict_get(in_opts, "bcd", NULL, 0);
}

int FFmpegMethods::av_input_video_device_next_test()
{
	avformat_network_init();
	avdevice_register_all();
	int argc = 4;
	char* argv[4];

	AVInputFormat *ifmt = NULL;
	char *dev = NULL;
	AVDictionary *opts = NULL;
	int ret = 0;
	int error_level = av_log_get_level();

	if ((ret = show_sinks_sources_parse_arg(argv[3], &dev, &opts)) < 0) {
		goto fail;
	}
	do {
		ifmt = av_input_video_device_next(ifmt);
		if (ifmt) {
			if (dev && !av_match_name(dev, ifmt->name))
				continue;
			print_device_sources(ifmt, opts);
		}
	} while (ifmt);

fail:
	av_dict_free(&opts);
	av_free(dev);
	av_log_set_level(error_level);
	return ret;
}

// arg:参数字符串
// dev:参数字符串的副本
// opts:从参数字符串解析出来的键值对,key和value用=区分,每个键值对试用:分割
int FFmpegMethods::show_sinks_sources_parse_arg(const char *arg, char **dev, AVDictionary **opts) {
	int ret;
	if (arg) {
		char *opts_str = NULL;
		*dev = av_strdup(arg);
		if (!*dev)
			return AVERROR(ENOMEM);
		if ((opts_str = strchr(*dev, ','))) {
			*(opts_str++) = '\0';
			if (opts_str[0] && ((ret = av_dict_parse_string(opts, opts_str, "=", ":", 0)) < 0)) {
				av_freep(dev);
				return ret;
			}
		}
	}
	else {
		printf("\nDevice name is not provided.\n"
			"You can pass devicename[,opt1=val1[,opt2=val2...]] as an argument.\n\n");
	}
	return 0;
}


int FFmpegMethods::print_device_sources(AVInputFormat *ifmt, AVDictionary *opts) {
	int ret, i;
	AVDeviceInfoList *device_list = NULL;

	if (!ifmt || !ifmt->priv_class || !AV_IS_INPUT_DEVICE(ifmt->priv_class->category)) {
		return AVERROR(EINVAL);
	}

	printf("Auto-detected sources for %s:\n", ifmt->name);
	if (!ifmt->get_device_list) {
		ret = AVERROR(ENOSYS);
		printf("Cannot list source,Not implemented.\n");
		goto fail;
	}

	if ((ret = avdevice_list_input_sources(ifmt, NULL, opts, &device_list)) < 0) {
		printf("Connot list sources.\n");
		goto fail;
	}

	for (i = 0; i < device_list->nb_devices; i++) {
		printf("%s %s [%s]\n", device_list->default_device == i ? "*" : " ",
			device_list->devices[i]->device_name, device_list->devices[i]->device_description);
	}

fail:
	avdevice_free_list_devices(&device_list);
	return ret;
}
