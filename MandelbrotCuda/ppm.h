#pragma once

#include "main.h"

#include <string>
#include <vector>
#include <fstream>

class writePPMFile {
private:
	std::string filename;
	std::vector<std::vector<uint32_t>> data;

public:
	error_t write_file()
	{
		std::ofstream mImage(filename);
		if (!mImage.is_open()) {
			return -1;
		}

		mImage << "P3\n" << data[0].size() << " " << data.size() << " " << " 255\n";
		for (std::vector<std::vector<uint32_t>>::const_iterator line = data.begin(); line != data.end(); ++line) {
			for (std::vector<uint32_t>::const_iterator column = line->begin(); column != line->end(); ++column) {
				
			}
		}

		return 0;
	}

	writePPMFile(std::string filename, std::vector<std::vector<uint32_t>> data) :
		filename(filename),
		data(data)
	{

	}

	~writePPMFile()
	{

	}
};

//EOF