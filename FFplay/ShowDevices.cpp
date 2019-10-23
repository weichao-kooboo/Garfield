#include "pch.h"
#include "ShowDevices.h"


ShowDevices::ShowDevices()
{
}


ShowDevices::~ShowDevices()
{
}

void ShowDevices::show_dshow_devices()
{
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVDictionary *options = NULL;
	av_dict_set(&options, "list_devices", "true", 0);
	AVInputFormat *iformat = av_find_input_format("dshow");
	printf("=============Device Info=================\n");
	// 显示所有输入设备
	avformat_open_input(&pFormatCtx, "video=dummy", iformat, &options);
	printf("=========================================\n");
}

void ShowDevices::show_dshow_device_options()
{
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVDictionary *options = NULL;
	av_dict_set(&options, "list_options", "true", 0);
	AVInputFormat *iformat = av_find_input_format("dshow");
	printf("=============Device Option Info==========\n");
	// video参数后面跟设备的名称
	avformat_open_input(&pFormatCtx, "video=XiaoMi USB 2.0 Webcam", iformat, &options);
	printf("=========================================\n");
}

void ShowDevices::show_vfw_device()
{
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVInputFormat *iformat = av_find_input_format("vfwcap");
	printf("=============VFW Device Info=============\n");
	avformat_open_input(&pFormatCtx, "list", iformat, NULL);
	printf("=========================================\n");
}

void ShowDevices::show_avfoundation_device()
{
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVDictionary *options = NULL;
	av_dict_set(&options, "list_devices", "true", 0);
	AVInputFormat *iformat = av_find_input_format("avfoundation");
	printf("=======AVFoundation Device Info==========\n");
	// 显示所有输入设备
	avformat_open_input(&pFormatCtx, "", iformat, &options);
	printf("=========================================\n");
}
