#ifndef __LOG_H__
#define __LOG_H__
#include "common.h"

BEGIN_NS

#define LOG_INFO(output) Log::getInstance().info(output)
#define LOG_WARNING(output) Log::getInstance().warning(output)

class Log
{
public:
	static Log& getInstance();

private:
	Log();

private:
	void time(char* in);

public:
	void info(const char* output);
	void warning(const char* output);
	void error(const char* output);
};

END_NS
#endif