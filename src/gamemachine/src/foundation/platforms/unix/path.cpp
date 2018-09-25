#include "stdafx.h"
#include "check.h"
#include <gmtools.h>
#include <dirent.h>

namespace
{
	int GetModuleFileName( char* sFileName, int nSize)
	{
		static char sLine[1024] = { 0 };
		int ret = -1;
		void* pSymbol = (void*)"";
		FILE *fp;
		char *pPath;

		fp = fopen ("/proc/self/maps", "r");
		if ( fp != NULL )
		{
			while (!feof (fp))
			{
				unsigned long start, end;

				if ( !fgets (sLine, sizeof (sLine), fp))
					continue;
				if ( !strstr (sLine, " r-xp ") || !strchr (sLine, '/'))
					continue;

				sscanf (sLine, "%lx-%lx ", &start, &end);
				if (pSymbol >= (void *) start && pSymbol < (void *) end)
				{
					char *tmp;
					size_t len;

					/* Extract the filename; it is always an absolute path */
					pPath = strchr (sLine, '/');

					/* Get rid of the newline */
					tmp = strrchr (pPath, '\n');
					if (tmp) *tmp = 0;

					/* Get rid of "(deleted)" */
					len = strlen (pPath);
					if (len > 10 && strcmp (pPath + len - 10, " (deleted)") == 0)
					{
						tmp = pPath + len - 10;
						*tmp = 0;
					}
					ret = 0;
					strcpy( sFileName, pPath );
				}
			}
			fclose (fp);
		
	 	}
		return ret;
	}

	void getAllFiles(Vector<GMString>& v, const std::string& directory, bool recursive)
	{
		DIR* dir = NULL;
		dirent* file = NULL;
		dir = opendir(directory.c_str());
		if (!dir)
			return;

		while ((file = readdir(dir)) != NULL)
		{
			if (file->d_type & DT_DIR)
			{
				if (GMString::stringEquals(file->d_name, ".") ||
					GMString::stringEquals(file->d_name, ".."))
				{
					continue;
				}

				if (recursive)
					::getAllFiles(v, GMPath::fullname(directory, file->d_name).toStdString(), recursive);
			}
			else
			{
				v.push_back(GMPath::fullname(directory, file->d_name));
			}
		}
		closedir(dir);
	}
}

GMString GMPath::directoryName(const GMString& fileName)
{
	GMString unixFileName = fileName.replace("\\", "/");
	GMsize_t pos = unixFileName.findLastOf('/');
	if (pos == GMString::npos)
		return fileName;
	return fileName.substr(0, pos + 1);
}

GMString GMPath::filename(const GMString& fullPath)
{
	GMString unixFileName = fullPath.replace("\\", "/");
	GMsize_t pos = unixFileName.findLastOf('/');
	if (pos == GMString::npos)
		return fullPath;
	return fullPath.substr(pos + 1, fullPath.length());
}

GMString GMPath::fullname(const GMString& dirName, const GMString& fullPath)
{
	GMString unixDirName = dirName.replace("\\", "/");
	GMsize_t pos = unixDirName.findLastOf('/');
	if (!unixDirName.isEmpty() && unixDirName[unixDirName.length() - 1] != L'/')
		return unixDirName + "/" + fullPath;
	return unixDirName + fullPath;
}

GMString GMPath::getCurrentPath()
{
	const int MAX = 255;
	char fn[MAX];
	::GetModuleFileName(fn, MAX);
	return directoryName(fn);
}

GMString GMPath::getSpecialFolderPath(SpecialFolder sf)
{
	switch (sf)
	{
	case GMPath::Fonts:
	{
		return GMString("/usr/share/fonts");
	}
	default:
		return GMString();
	}
}

Vector<GMString> GMPath::getAllFiles(const GMString& directory, bool recursive)
{
	Vector<GMString> res;
	std::string d = directory.toStdString();
	::getAllFiles(res, d, recursive);
	return res;
}
