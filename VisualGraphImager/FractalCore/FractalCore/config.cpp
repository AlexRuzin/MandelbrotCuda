#include "config.h"

#include "types.h"
#include "debug.h"

#include <string>
#include <iostream>
#include <filesystem>
#include <INIReader.h>

using namespace config;

error_t iniParser::load_config_file(void)
{
	if (iniParser::check_if_file_exists(fileName) == false) {
		DERROR("File does not exist: " + fileName);
		return -1;
	}

	reader = new INIReader(fileName);
	if (reader->ParseError() != 0) {
		DERROR("Failed to parse file: " + fileName);
		return -1;
	}



	return 0;
}

bool iniParser::check_if_file_exists(std::string filename)
{
	const std::filesystem::path path(filename);
	return std::filesystem::exists(path);
}