#include "stdafx.h"
#include "check.h"
#include <gmtools.h>
#include <io.h>
#include <direct.h>
#include <shlobj.h>

namespace
{
	void getAllFiles(Vector<GMString>& v, const GMString& directory, bool recursive)
	{
		WIN32_FIND_DATA findFileData;
		GMString wildcard = GMPath::fullname(directory, "*");
		const std::wstring& wildcardStr = wildcard.toStdWString();
		HANDLE hFind = FindFirstFile(wildcardStr.c_str(), &findFileData);
		if (!GMString::stringEquals(L".", findFileData.cFileName) &&
			!GMString::stringEquals(L"..", findFileData.cFileName))
		{
			if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				v.push_back(GMPath::fullname(directory, findFileData.cFileName));
		}

		while (FindNextFile(hFind, &findFileData))
		{
			if (!GMString::stringEquals(L".", findFileData.cFileName) &&
				!GMString::stringEquals(L"..", findFileData.cFileName))
			{
				if (recursive && findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					getAllFiles(v, directory, recursive);
				}
				else
				{
					v.push_back(GMPath::fullname(directory, findFileData.cFileName));
				}
			}
		}

		FindClose(hFind);
	}
}

GMString GMPath::directoryName(const GMString& fileName)
{
	GMString winFileName = fileName.replace("/", "\\");
	GMsize_t pos = winFileName.findLastOf('\\');
	if (pos == GMString::npos)
		return fileName;
	return fileName.substr(0, pos + 1);
}

GMString GMPath::filename(const GMString& fullPath)
{
	GMString winFileName = fullPath.replace("/", "\\");
	GMsize_t pos = winFileName.findLastOf('\\');
	if (pos == GMString::npos)
		return fullPath;
	return fullPath.substr(pos + 1, fullPath.length());
}

GMString GMPath::fullname(const GMString& dirName, const GMString& fullPath)
{
	GMString winDirName = dirName.replace("/", "\\");
	GMsize_t pos = winDirName.findLastOf('\\');
	if (!winDirName.isEmpty() && winDirName[winDirName.length() - 1] != L'\\')
		return dirName + "\\" + fullPath;
	return dirName + fullPath;
}

GMString GMPath::getCurrentPath()
{
	const int MAX = 255;
	GMwchar fn[MAX];
	::GetModuleFileName(NULL, fn, MAX);
	return directoryName(fn);
}

bool GMPath::fileExists(const GMString& dir)
{
	WIN32_FIND_DATA findFileData;
	HANDLE hFind;
	hFind = FindFirstFile(dir.toStdWString().c_str(), &findFileData);

	bool b = hFind != INVALID_HANDLE_VALUE;
	FindClose(hFind);
	return b;
}

void GMPath::createDirectory(const GMString& dir)
{
	if (fileExists(dir) || (dir.length() == 2 && dir.toStdWString()[1] == L':'))
		return;

	std::wstring stdUp = dir.toStdWString();
	if (stdUp.back() == '/' || stdUp.back() == '\\')
		stdUp = stdUp.substr(0, stdUp.length() - 1); //去掉斜杠和反斜杠
	stdUp = directoryName(stdUp).toStdWString();
	for (GMuint32 i = 0; i < stdUp.length(); i++)
	{
		if (stdUp[i] == '/')
			stdUp[i] = '\\';
	}
	if (stdUp.back() == '/' || stdUp.back() == '\\')
		stdUp = stdUp.substr(0, stdUp.length() - 1);

	createDirectory(stdUp);
	std::string strDir = GMString(dir).toStdString();
	_mkdir(strDir.c_str());
}

GMString GMPath::getSpecialFolderPath(SpecialFolder sf)
{
	switch (sf)
	{
	case GMPath::Fonts:
	{
		GMwchar path[MAX_PATH];
		SHGetSpecialFolderPath(NULL, path, CSIDL_FONTS, FALSE);
		return GMString(path);
	}
	default:
		return GMString();
	}
}