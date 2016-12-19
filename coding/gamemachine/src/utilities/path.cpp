#include "stdafx.h"
#include "path.h"

std::string Path::directoryName(const std::string& fileName)
{
	int pos = fileName.find_last_of('\\');
	if (pos == std::string::npos)
		return fileName;
	return fileName.substr(0, pos + 1);
}

std::string Path::getCurrentPath()
{
#if _WINDOWS
	const int MAX = 255;
	CHAR fn[MAX];
	::GetModuleFileName(NULL, fn, MAX);
	return directoryName(fn);
#endif
	return "";
}