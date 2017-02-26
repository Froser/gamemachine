#ifndef __PATH_H__
#define __PATH_H__
#include "common.h"
#include <string>
#include <vector>

BEGIN_NS
struct Path
{
	static std::string directoryName(const std::string& fileName);
	static std::string getCurrentPath();
	static std::vector<std::string> getAllFiles(const char* directory);
};
END_NS
#endif