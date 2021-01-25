#pragma once

#include <string>

#include "main.h"
#include "types.h"

namespace ui {
	class window {
	private:
		const std::string windowResourceName;

	public:
		window(std::string windowResourceName) :
			windowResourceName(windowResourceName)
		{
			
		}

		error_t init_window(int argc, char *argv[]);
	};
}