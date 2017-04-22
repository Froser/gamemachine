#ifndef __PATH_H__
#define __PATH_H__
#include "common.h"
#include <string>
#include <vector>

BEGIN_NS
struct Path
{
	static std::string directoryName(const std::string& fileName);
	static std::string filename(const std::string& fullPath);
	static std::string getCurrentPath();
	static std::vector<std::string> getAllFiles(const char* directory);
	static bool directoryExists(const std::string& dir);
	static void createDirectory(const std::string& dir);
};
END_NS
#endif