#include "stdafx.h"
#include "path.h"
#include <algorithm>
#ifdef _WINDOWS
#include <io.h>
#include <direct.h>
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

std::string Path::filename(const std::string& fullPath)
{
	int pos1 = fullPath.find_last_of('\\'),
		pos2 = fullPath.find_last_of('/');
	int pos = pos1 > pos2 ? pos1 : pos2;
	if (pos == std::string::npos)
		return fullPath;
	return fullPath.substr(pos + 1, fullPath.size());
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
			if ((d.attrib &  _A_ARCH))
			{
				if (!strEqual(d.name, ".") && !strEqual(d.name, ".."))
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

bool Path::directoryExists(const std::string& dir)
{
#ifdef _WINDOWS
	WIN32_FIND_DATA findFileData;
	HANDLE hFind;
	hFind = FindFirstFile(dir.c_str(), &findFileData);

	bool b = hFind != INVALID_HANDLE_VALUE;
	FindClose(hFind);
	return b;
#else
#error need implement
#endif
}

void Path::createDirectory(const std::string& dir)
{
#ifdef _WINDOWS
	if (directoryExists(dir) || (dir.size() == 2 && dir[1] == ':'))
		return;

	std::string up = dir;
	if (up.back() == '/' || up.back() == '\\')
		up = up.substr(0, up.size() - 1); //去掉斜杠和反斜杠
	up = directoryName(up);
	for (GMuint i = 0; i < up.size(); i++)
	{
		if (up[i] == '/')
			up[i] = '\\';
	}
	if (up.back() == '/' || up.back() == '\\')
		up = up.substr(0, up.size() - 1);

	createDirectory(up);
	_mkdir(dir.c_str());
#else
#error need implement
#endif
}