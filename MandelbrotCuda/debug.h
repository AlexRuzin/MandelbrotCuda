#pragma once

#if defined(_WIN32)
#include <Windows.h>
#endif //__WIN32

#if !defined(DEBUG_LIBRARY_INCLUDED)
#define DEBUG_LIBRARY_INCLUDED
#else //DEBUG_LIBRARY_INCLUDED
#pragma message("DEBUG_LIBRARY_INCLUDED already defined")
#endif //DEBUG_LIBRARY_INCLUDED

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <assert.h>
#include <fstream>
#include <ctime>
#include <cstdio>
#include <stdio.h>
#include <mutex>

#undef DISABLE_DEBUG_LOGGING

#undef USE_COMPLEX_DEBUGGING
#if defined(USE_COMPLEX_DEBUGGING)
#define DINFO(s) debug::d->debugPrint(debug::DEBUG_LEVEL_INFO, s)
#define DERROR(s) debug::d->debugPrint(debug::DEBUG_LEVEL_ERROR, s)
#define DWARNING(s) debug::d->debugPrint(debug::DEBUG_LEVEL_WARNING, s)
#define DDEBUG(s) debug::d->debugPrint(debug::DEBUG_LEVEL_DEBUG, s)
#endif //USE_COMPLEX_DEBUGGING

#if !defined(USE_COMPLEX_DEBUGGING)
void debugPrint(std::string level, std::string s);
#define DINFO(s) debugPrint("INFO", s)
#define DERROR(s) debugPrint("ERROR", s)
#define DWARNING(s) debugPrint("WARN", s)
#define DDEBUG(s) debugPrint("DEBUG", s)
#endif

#if defined(USE_COMPLEX_DEBUGGING)
namespace debug
{
	extern "C" {
	class debugOut;
	extern debugOut *d;

	enum {
		DEBUG_LEVEL_INFO,
		DEBUG_LEVEL_ERROR,
		DEBUG_LEVEL_WARNING,
		DEBUG_LEVEL_DEBUG
	};

	typedef uint32_t debugLevel;
	typedef std::map<debugLevel, std::string> debugLevels;
	extern debugLevels debug = {
		{DEBUG_LEVEL_INFO, "INFO"},
		{DEBUG_LEVEL_ERROR, "ERROR"},
		{DEBUG_LEVEL_WARNING, "WARN"},
		{DEBUG_LEVEL_DEBUG, "DEBUG"}
	};

	typedef unsigned long debugFlags;
#define DEBUG_FLAG_STDOUT			0x00000001 // Debug output to stdout
#define DEBUG_FLAG_LOGFILE			0x00000002 // Debug output to log file
#define DEBUG_FLAG_CLEANLOGFILE		0x00000004 // Clears the debug log file
#define DEBUG_FLAG_TIMESTAMP		0x00000008 // Adds a timestamp to the log

	class debugOut {
	private:
		std::vector<std::string> log;
		std::ofstream* outfile;

		const debugFlags flags;
		const std::string *logFilename;

		std::mutex writeSync;

	public:
		debugOut(const std::string *filename, debugFlags flags) :
			logFilename(filename), outfile(nullptr), flags(flags)
		{
#if defined(DISABLE_DEBUG_LOGGING)
			return;
#endif //DISABLE_DEBUG_LOGGING

			if (DEBUG_FLAG_CLEANLOGFILE & flags) {
				assert(filename != nullptr);
				std::remove(const_cast<std::string&>(*filename).c_str());
			}
		
			if (DEBUG_FLAG_LOGFILE & flags) {
				outfile = new std::ofstream(*filename);
			}

			debugPrint(DEBUG_LEVEL_INFO, "Starting debug logging");
		}

		~debugOut()
		{
			if (outfile != nullptr) {
				delete outfile;
			}
		}

		void debugPrint(debugLevel level, const std::string s);
	};

	extern debugOut* init_debug(const std::string* filename, debugFlags flags);

	extern void sleep(uint32_t ms);

	} // Extern C
}
#endif //USE_COMPLEX_DEBUGGING

//EOF