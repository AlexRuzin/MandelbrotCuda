#include "debug.h"

#include <string>

#if defined(USE_COMPLEX_DEBUGGING)
using namespace debug;

void debugOut::debugPrint(debugLevel level, const std::string s)
{
	assert(debug.find(level) != debug.end());

	this->writeSync.lock();

#if defined(DISABLE_DEBUG_LOGGING)
	return;
#endif //DISABLE_DEBUG_LOGGING

	std::string ss;
	if (DEBUG_FLAG_TIMESTAMP & flags) {
		time_t rawtime;
		struct tm *timeinfo;
		char buffer[80] = { 0 };

		std::time(&rawtime);
		timeinfo = std::localtime(&rawtime);

		std::strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
		std::string str(buffer);

		ss = "[" + str + "]\t" + "[" + debug[level] + "] " + s + "\n";
	}
	else {
		ss = "[" + debug[level] + "] " + s + "\n";
	}


	if (DEBUG_FLAG_LOGFILE & flags) {
		*outfile << ss;
	}

	log.push_back(ss);

	if (DEBUG_FLAG_STDOUT & flags) {
#if defined(_WIN32)
		OutputDebugStringA(ss.c_str());
#else  //_WIN32
		std::cout << ss << std::endl;
#endif //_WIN32
	}
}

extern debugOut *init_debug(const std::string *filename, debugFlags flags)
{
	debugOut *o = new debugOut(filename, flags);
	d = o;
	return o;
}

extern void sleep(uint32_t ms)
{
#if defined(_WIN32)
	Sleep(ms);
#else //_WIN32

#endif //_WIN32
}
#else //USE_COMPLEX_DEBUGGING
void debugPrint(std::string level, std::string s)
{
	std::string ss;

	time_t rawtime;
	struct tm *timeinfo;
	char buffer[80] = { 0 };

	std::time(&rawtime);
	timeinfo = std::localtime(&rawtime);

	std::strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
	std::string str(buffer);

	ss = "[" + str + "]\t" + "[" + level + "] " + s + "\n";
	OutputDebugStringA(ss.c_str());
}
#endif

//EOF