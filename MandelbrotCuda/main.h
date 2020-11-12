#pragma once

//#include "stdafx.h"
#include <stdint.h>
#include <string>
#include <iostream>

typedef int32_t error_t;

void DEBUG_ERR(std::string s)
{
	std::cout << "[err]\t" << s << std::endl;
}

void DEBUG_INFO(std::string s)
{
	std::cout << "[info]\t" << s << std::endl;
}

//EOF