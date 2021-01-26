#pragma once

//#define _WIN32
#if defined(_WIN32)
#include "windows.h"
#endif //__WIN32

#include <stdint.h>

#include "main.h"
#include "config.h"

uint32_t main(uint32_t argc, char *argv[])
{
	const std::string config_file(PRIMARY_CONFIG_FILE);
	config::iniParser parser(config_file);
	if (error_t err = parser.load_config_file() != 0) {
		return err;
	}



	return 0;
}
