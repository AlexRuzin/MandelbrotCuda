#pragma once

#include <string>
#include <assert.h>
#include <stdint.h>
#include <INIReader.h>

#include "main.h"
#include "types.h"

namespace config {
	typedef struct iniContents {
		typedef struct {
			/*
			 * CUDA configuration
			 */

			 // Size of the CUDA frame buffer
			uint32_t	FRAME_BUFFER_HEIGHT;
			uint32_t	FRAME_BUFFER_LENGTH;

			// Initial offset of C for the fractal object
			double		INITIAL_OFFSET_X;
			double		INITIAL_OFFSET_Y;

			// milliseconds
			uint32_t	CONTROLLER_LOOP_WAIT;

			// Total number of iterations for the fractal generator
			uint32_t	MAX_FRACTAL_ITERATIONS;

			// CUDA initial scale parameters
			double		IMAGE_SCALEA;
			double		IMAGE_SCALEB;

			// Zoom scale rate
			double		ZOOM_ALPHA;
			double		ZOOM_BETA;

			// Scale and position delta (this is probably obsolete)
			double		DELTA_SCALEA;
			double		DELTA_SCALEB;

			// Do not exceed this Delta Scale A (i.e cause a div by 0 on scale zoom out)
			double		MAX_DELTA_SCALE_A;
		} CUDA, *PCUDA;
		CUDA cuda;
		
		/*
		 * SDL2 configuration
		 */

		// SDL2 window dimensions
		typedef struct {
			uint32_t	SDL2_WINDOW_HEIGHT;
			uint32_t	SDL2_WINDOW_LENGTH;

			std::string	SDL2_WINDOW_NAME;

			uint8_t		CROSSHAIR_COLOR[3];
		} SDL2, *PSDL2;
		SDL2 sdl2;

		std::string	DEBUG_LOG_FILE;

	} INI_CONTENTS, *PINI_CONTENTS;

#define SET_PARSER
	class iniParser {
	private:
		const std::string fileName;

		const std::string cuda_section_name;
		const std::string sdl2_section_name;

		INI_CONTENTS iniData;

		INIReader *reader;

	public:
		iniParser(std::string fileName) :
			fileName(fileName),
			cuda_section_name("CUDA_Config"),
			sdl2_section_name("SDL2_Config")
		{
			iniData = { 0 };
		}

		~iniParser(void)
		{
			if (reader != nullptr) {
				delete reader;
			}
		}

		// Parse the file
		error_t parse_config_file(void);

		// Return the config file
		const INI_CONTENTS *get_config(void) const
		{
			return const_cast<const INI_CONTENTS *>(&iniData);
		}


		static bool check_if_file_exists(std::string filename);
	};
}