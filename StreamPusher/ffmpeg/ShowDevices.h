#pragma once
/* FFmpeg中有一个和多媒体设备交互的类库：Libavdevice。
 * 使用这个库可以读取电脑（或者其他设备上）的多媒体设备的数据，或者输出数据到指定的多媒体设备上
 *
 * 输入端设备包括:
	alsa
	avfoundation
	bktr
	dshow
	dv1394
	fbdev
	gdigrab
	iec61883
	jack
	lavfi
	libcdio
	libdc1394
	openal
	oss
	pulse
	qtkit
	sndio
	video4linux2, v4l2
	vfwcap
	x11grab
	decklink

 * 输出端设备包括:
	alsa
	caca
	decklink
	fbdev
	opengl
	oss
	pulse
	sdl
	sndio
	xv
 *
 * 该类用于获取连接到PC端的设备,和设备的参数
 * 参考:https://blog.csdn.net/leixiaohua1020/article/details/39702113
*/
#ifndef _SHOW_DEVICES_H_INCLUDED_
#define	_SHOW_DEVICES_H_INCLUDED_

#include "ffmpegHeader.h"

class ShowDevices
{
public:
	ShowDevices();
	~ShowDevices();

	// show DShow Devices
	static void show_dshow_devices();

	//show DShow device option
	static void show_dshow_device_options();

	//show VFW device
	static void show_vfw_device();

	//show AVFoundation Device
	static void show_avfoundation_device();
};
#endif // !_SHOW_DEVICES_H_INCLUDED_

