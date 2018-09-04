#include "stdafx.h"
#include "check.h"
#include <gmtools.h>
#include <io.h>
#include <direct.h>
#include <shlobj.h>

//TODO 这里在其他系统下存在问题
BOOL SHGetSpecialFolderPathA(HWND hwnd, LPSTR pszPath, int csidl, BOOL fCreate);
BOOL SHGetSpecialFolderPathW(HWND hwnd, LPWSTR pszPath, int csidl, BOOL fCreate);
#	ifdef UNICODE
#		define SHGetSpecialFolderPath  SHGetSpecialFolderPathW
#	else
#		define SHGetSpecialFolderPath  SHGetSpecialFolderPathA
#	endif // !UNICODE
#	define CSIDL_FONTS 0x0014
#endif

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

Vector<GMString> GMPath::getAllFiles(const GMString& directory)
{
	Vector<GMString> res;
	GMString p = directory;
	p.append("*");
	_finddata_t fd;
	long hFile = 0;
	if ((hFile = _findfirst(p.toStdString().c_str(), &fd)) != -1)
	{
		do
		{
			if ((fd.attrib &  _A_ARCH))
			{
				if (!GMString::stringEquals(fd.name, ".") && !GMString::stringEquals(fd.name, ".."))
					res.push_back(GMString(directory).append(fd.name));
			}
		} while (_findnext(hFile, &fd) == 0);
		_findclose(hFile);
	}
	return res;
}

bool GMPath::directoryExists(const GMString& dir)
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
	if (directoryExists(dir) || (dir.length() == 2 && dir.toStdWString()[1] == L':'))
		return;

	std::wstring stdUp = dir.toStdWString();
	if (stdUp.back() == '/' || stdUp.back() == '\\')
		stdUp = stdUp.substr(0, stdUp.length() - 1); //去掉斜杠和反斜杠
	stdUp = directoryName(stdUp).toStdWString();
	for (GMuint i = 0; i < stdUp.length(); i++)
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