#include "stdafx.h"
#include "path.h"

std::string Path::directoryName(const std::string& fileName)
{
	int pos = fileName.find_last_of('\\');
	if (pos == std::string::npos)
		return fileName;
	return fileName.substr(0, pos + 1);
}