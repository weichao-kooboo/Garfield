#pragma once
#ifndef _SP_CONF_FILE_H_INCLUDED_
#define _SP_CONF_FILE_H_INCLUDED_

#include "sp_core.h"

struct sp_open_file_s{
	sp_fd_t		fd;
	sp_str_t	name;
	void(*flush)(sp_open_file_t *file, sp_log_t *log);
	void                 *data;
};

#endif // !_SP_CONF_FILE_H_INCLUDED_
