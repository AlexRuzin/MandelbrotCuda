#include "config.h"

#include "types.h"
#include "debug.h"

#include <string>
#include <iostream>
#include <filesystem>
#include <INIReader.h>

using namespace config;

error_t iniParser::parse_config_file(void)
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

	/*
	 * CUDA section
	 */
	iniData.cuda.FRAME_BUFFER_HEIGHT = reader->GetInteger(cuda_section_name, "frame_buffer_height", 0);
	iniData.cuda.FRAME_BUFFER_LENGTH = reader->GetInteger(cuda_section_name, "frame_buffer_length", 0);
	
	iniData.cuda.INITIAL_OFFSET_X = reader->GetReal(cuda_section_name, "initial_offset_x", -1);
	iniData.cuda.INITIAL_OFFSET_Y = reader->GetReal(cuda_section_name, "initial_offset_y", -1);

	// Halt the loop in case of a misconfig
	iniData.cuda.CONTROLLER_LOOP_WAIT = reader->GetInteger(cuda_section_name, "controller_loop_wait", 100000);

	iniData.cuda.MAX_FRACTAL_ITERATIONS = reader->GetInteger(cuda_section_name, "max_fractal_iterations", 0);

	iniData.cuda.IMAGE_SCALEA = reader->GetReal(cuda_section_name, "image_scaleA", 0.0);
	iniData.cuda.IMAGE_SCALEB = reader->GetReal(cuda_section_name, "image_scaleB", 0.0);

	iniData.cuda.ZOOM_ALPHA = reader->GetReal(cuda_section_name, "zoom_alpha", 0.0);
	iniData.cuda.ZOOM_BETA = reader->GetReal(cuda_section_name, "zoom_beta", 0.0);

	iniData.cuda.DELTA_SCALEA = reader->GetReal(cuda_section_name, "delta_scaleA", 0.0);
	iniData.cuda.DELTA_SCALEB = reader->GetReal(cuda_section_name, "delta_scaleB", 0.0);

	iniData.cuda.MAX_DELTA_SCALE_A = reader->GetReal(cuda_section_name, "max_delta_scaleA", 0.0);


	/*
	 * SDL2 section
	 */
	iniData.sdl2.SDL2_WINDOW_HEIGHT = reader->GetInteger(sdl2_section_name, "sdl2_window_height", 0);
	iniData.sdl2.SDL2_WINDOW_LENGTH = reader->GetInteger(sdl2_section_name, "sdl2_window_length", 0);

	iniData.sdl2.SDL2_WINDOW_NAME = reader->GetString(sdl2_section_name, "sdl2_window_name", "NULL");

	iniData.sdl2.CROSSHAIR_COLOR[0] = (uint8_t)reader->GetInteger(sdl2_section_name, "crosshair_red", 0);
	iniData.sdl2.CROSSHAIR_COLOR[1] = (uint8_t)reader->GetInteger(sdl2_section_name, "crosshair_green", 0);
	iniData.sdl2.CROSSHAIR_COLOR[2] = (uint8_t)reader->GetInteger(sdl2_section_name, "crosshair_blue", 0);

	return 0;
}

bool iniParser::check_if_file_exists(std::string filename)
{
	const std::filesystem::path path(filename);
	return std::filesystem::exists(path);
}