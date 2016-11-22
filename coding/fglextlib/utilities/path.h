#ifndef __PATH_H__
#include "common.h"
#include <string>
BEGIN_NS
struct Path
{
	static std::string directoryName(const std::string& fileName);
	static std::string getCurrentPath();
};
END_NS
#endif