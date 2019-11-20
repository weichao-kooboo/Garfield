#include "RtmpPusher.h"

RtmpPusher::RtmpPusher()
{

}

RtmpPusher::~RtmpPusher()
{
}

void RtmpPusher::push(const char * input_name, const char * output_name)
{

}

int RtmpPusher::openInput()
{
	int ret = 0;
	if ((ret = avformat_open_input(&_ifmt_ctx, _input_name, 0, 0)) < 0) {
		return;
	}
}

void RtmpPusher::openOutput()
{
}

void RtmpPusher::pushStream()
{
}
