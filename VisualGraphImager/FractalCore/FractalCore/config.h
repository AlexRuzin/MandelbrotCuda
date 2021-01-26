#pragma once

#include <string>
#include <stdint.h>
#include <INIReader.h>

#include "main.h"
#include "types.h"

namespace config {
	typedef struct iniContents {
		struct cudaController {
			// Size of the CUDA frame buffer
			const uint32_t		FRAME_BUFFER_HEIGHT;
			const uint32_t		FRAME_BUFFER_LENGTH;

			// CUDA initial scale parameters
			const double		IMAGE_SCALEA;
			const double		IMAGE_SCALEB;

			// Zoom scale rate
			const double		ZOOM_ALPHA;
			const double		ZOOM_BETA;

			// Scale and position delta
			const double		DELTA_SCALEA;
			const double		DELTA_SCALEB;

			// Do not exceed this Delta Scale A (i.e cause a div by 0 on scale zoom out)
			const double		MAX_DELTA_SCALE_A;

			// milliseconds
			const uint32_t		CONTROLLER_LOOP_WAIT;
		};

		struct sdl2 {
			// SDL2 window dimensions
			const uint32_t		SDL2_WINDOW_HEIGHT;
			const uint32_t		SDL2_WINDOW_LENGTH;

			const std::string	SDL2_WINDOW_NAME;

			const uint8_t		CROSSHAIR_COLOR[3];
		};

		const std::string		DEBUG_LOG_FILE;
	} INI_CONTENTS, *PINI_CONTENTS;

	class iniParser {
	private:
		const std::string fileName;

		INI_CONTENTS *data;

		INIReader *reader;

	public:
		iniParser(std::string fileName) :
			data(nullptr),
			fileName(fileName)
		{

		}

		~iniParser(void)
		{
			if (reader != nullptr) {
				delete reader;
			}
		}

		error_t load_config_file(void);

		static bool check_if_file_exists(std::string filename);
	};
}