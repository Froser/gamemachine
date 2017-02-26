#include "stdafx.h"
#include "path.h"
#ifdef _WINDOWS
#include <io.h>
#endif

std::string Path::directoryName(const std::string& fileName)
{
	int pos1 = fileName.find_last_of('\\'),
		pos2 = fileName.find_last_of('/');
	int pos = pos1 > pos2 ? pos1 : pos2;
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

std::vector<std::string> Path::getAllFiles(const char* directory)
{
	std::vector<std::string> res;
#if _WINDOWS
	std::string p = directory;
	p.append("*");
	_finddata_t d;
	long hFile = 0;
	if ((hFile = _findfirst(p.c_str(), &d)) != -1)
	{
		do
		{
			if ((d.attrib &  _A_NORMAL))
			{
				if (strcmp(d.name, ".") != 0 && strcmp(d.name, "..") != 0)
					res.push_back(std::string(directory).append(d.name));
			}
		} while (_findnext(hFile, &d) == 0);
		_findclose(hFile);
	}
#else
#error need implement
#endif
	return res;
}