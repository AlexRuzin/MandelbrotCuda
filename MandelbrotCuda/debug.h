#pragma once

#if defined(_WIN32)
#include <Windows.h>
#endif //__WIN32

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

#define DINFO(s) debug::d->debugPrint(debug::DEBUG_LEVEL_INFO, s)
#define DERROR(s) debug::d->debugPrint(debug::DEBUG_LEVEL_ERROR, s)
#define DWARNING(s) debug::d->debugPrint(debug::DEBUG_LEVEL_WARNING, s)
#define DDEBUG(s) debug::d->debugPrint(debug::DEBUG_LEVEL_DEBUG, s)

namespace debug
{
	class debugOut;
	debugOut *d;

	enum {
		DEBUG_LEVEL_INFO,
		DEBUG_LEVEL_ERROR,
		DEBUG_LEVEL_WARNING,
		DEBUG_LEVEL_DEBUG
	};

	typedef uint32_t debugLevel;
	typedef std::map<debugLevel, std::string> debugLevels;
	debugLevels debug = {
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

		void debugPrint(debugLevel level, const std::string s)
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
			} else {
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
			this->writeSync.unlock();
		}
	};

	debugOut* init_debug(const std::string* filename, debugFlags flags)
	{
		debugOut *o = new debugOut(filename, flags);
		d = o;
		return o;
	}

	void sleep(uint32_t ms)
	{
#if defined(_WIN32)
		Sleep(ms);
#else //_WIN32

#endif //_WIN32
	}
}

//EOF