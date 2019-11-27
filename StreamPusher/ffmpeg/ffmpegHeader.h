#pragma once
#ifndef _SP_FFMPEG_FFMPEGHEADER_H_INCLUDED_
#define _SP_FFMPEG_FFMPEGHEADER_H_INCLUDED_

extern	"C" {
#include "../core/export.h"
#include "../include/libavcodec/avcodec.h"
#include "../include/libavformat/avformat.h"
#include "../include/libavdevice/avdevice.h"
#include "../include/libavutil/time.h"
#include "../include/libavutil/avstring.h"
}
#include <memory>
#include <string>

#endif // !_SP_FFMPEG_FFMPEGHEADER_H_INCLUDED_
