#pragma once
#include "win_files.h"

ssize_t sp_write_console(sp_fd_t fd, void *buf, size_t size) {
	u_long  n;

	(void)CharToOemBuff(buf, buf, size);

	if (WriteFile(fd, buf, size, &n, NULL) != 0) {
		return (size_t)n;
	}

	return -1;
}