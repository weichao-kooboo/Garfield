#pragma once
/* FFmpeg����һ���Ͷ�ý���豸��������⣺Libavdevice��
 * ʹ���������Զ�ȡ���ԣ����������豸�ϣ��Ķ�ý���豸�����ݣ�����������ݵ�ָ���Ķ�ý���豸��
 *
 * ������豸����:
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

 * ������豸����:
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
 * �������ڻ�ȡ���ӵ�PC�˵��豸,���豸�Ĳ���
 * �ο�:https://blog.csdn.net/leixiaohua1020/article/details/39702113
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

