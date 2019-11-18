#pragma once
class AvioReading
{
public:
	AvioReading();
	~AvioReading();
	int run(int argc, char *argv[]);
private:
	int read_packet(void *opaque, uint8_t *buf, int buf_size);
};

