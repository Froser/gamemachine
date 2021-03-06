﻿#include "stdafx.h"
#include <fstream>
#include "gmgamepackagehandler.h"
#include "foundation/utilities/tools.h"
#include "foundation/gamemachine.h"
#include "foundation/gmasync.h"
#include "gmgamepackage.h"
#include "gmgamepackage_p.h"

BEGIN_NS

#define PKD(d) GMGamePackage::Data* d = gamePackage()->gamePackageData();

namespace
{
	template <class char_t>
	char_t toLower(char_t in)
	{
		return (in >= (char_t)'A' && in <= (char_t)'Z') ? (char_t)(in + 0x20) : in;
	}

	GMString toLower(const GMString& in)
	{
		GMString result;
		result.reserve(in.length());
		for (GMsize_t i = 0; i < in.length(); ++i)
		{
			result += toLower(in[i]);
		}
		return result;
	}

	GMString normalizePath(const GMString& in)
	{
		// 标准化一个路径： 先将所有字符串小写，将所有\替换为/，重叠的/替换为/，如果路径由/打头，则去掉这个字符
		if (in == L"/")
			return in;

		GMString s1 = toLower(in).replace("\\", "/");
		GMString s2;
		s2.reserve(s1.length());
		bool flag = false;

		// 当前一个字符为/时，flag置为true
		for (GMsize_t i = 0; i < s1.length(); ++i)
		{
			GMwchar ch = s1[i];
			if (flag)
			{
				if (ch != L'/')
				{
					flag = false;
					s2 += ch;
				}
			}
			else
			{
				if (ch == L'/')
					flag = true;
				s2 += ch;
			}
		}

		if (!s2.isEmpty() && s2[0] != L'/')
			return s2;

		return s2.substr(1, s2.length() - 1);
	}

}

GMDefaultGamePackageHandler::GMDefaultGamePackageHandler(GMGamePackage* pk)
	: m_pk(pk)
	, m_packageIndex(0)
{

}

bool GMDefaultGamePackageHandler::readFileFromPath(const GMString& path, REF GMBuffer* buffer)
{
	std::ifstream file;
	std::string p = path.toStdString();
	file.open(p, std::ios::in | std::ios::binary | std::ios::ate);
	if (file.good())
	{
		GMint32 size = file.tellg();
		if (size == -1)
		{
			gm_warning(gm_dbg_wrap("cannot read file from path: {0}"), path);
			return false;
		}

		buffer->resize(size);
		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(buffer->getData()), size);
		file.close();
		return true;
	}
	else
	{
		gm_warning(gm_dbg_wrap("cannot read file from path: {0}"), path);
	}
	return false;
}

GMString GMDefaultGamePackageHandler::pathOf(GMPackageIndex index, const GMString& fileName)
{
	GMMutexLock m(&m_mutex);
	m->lock();
	resetPackageCandidate();
	GMString result;
	while (!GMPath::fileExists(result = pathRoot(index) + fileName))
	{
		if (!nextPackageCandidate())
			break;
	}
	resetPackageCandidate();
	return result;
}

bool GMDefaultGamePackageHandler::exists(GMPackageIndex index, const GMString& fileName)
{
	GMMutexLock m(&m_mutex);
	m->lock();
	resetPackageCandidate();
	GMString result;
	while (!GMPath::fileExists(result = pathRoot(index) + fileName))
	{
		if (!nextPackageCandidate())
			return false;
	}
	resetPackageCandidate();
	return true;
}

void GMDefaultGamePackageHandler::init()
{
}

GMString GMDefaultGamePackageHandler::pathRoot(GMPackageIndex index)
{
	PKD(d);

	switch (index)
	{
	case GMPackageIndex::Root:
		return packagePath() + L"/";
	case GMPackageIndex::Maps:
		return packagePath() + L"/maps/";
	case GMPackageIndex::Shaders:
		return packagePath() + L"/shaders/";
	case GMPackageIndex::Textures:
		return packagePath() + L"/textures/";
	case GMPackageIndex::Models:
		return packagePath() + L"/models/";
	case GMPackageIndex::Audio:
		return packagePath() + L"/audio/";
	case GMPackageIndex::Particle:
		return packagePath() + L"/particles/";
	case GMPackageIndex::Scripts:
		return packagePath() + L"/scripts/";
	case GMPackageIndex::Fonts:
		return packagePath() + L"/fonts/";
	case GMPackageIndex::Effects:
		return packagePath() + L"/effects/";
	case GMPackageIndex::Prefetch:
		return packagePath() + L"/prefetch/";
	default:
		GM_ASSERT(false);
		break;
	}
	return "";
}

GMGamePackage* GMDefaultGamePackageHandler::gamePackage()
{
	return m_pk;
}

GMString GMDefaultGamePackageHandler::packagePath()
{
	PKD(d);
	if (m_packageIndex == 0)
		return d->packagePath;

	return d->packagePath + GMString(m_packageIndex);
}

void GMDefaultGamePackageHandler::resetPackageCandidate()
{
	m_packageIndex = 0;
}

bool GMDefaultGamePackageHandler::nextPackageCandidate()
{
	++m_packageIndex;
	if (!GMPath::fileExists(packagePath()))
		return false;
	return true;
}

#define CHECK(err) if (err != UNZ_OK) return false

GMZipGamePackageHandler::GMZipGamePackageHandler(GMGamePackage* pk)
	: GMDefaultGamePackageHandler(pk)
	, m_packageIndex(0)
	, m_packageCount(1)
{
}

void GMZipGamePackageHandler::init()
{
	PKD(d);
	if (!loadZip())
	{
		gm_error(gm_dbg_wrap("invalid package file {0}"), d->packagePath);
		return;
	}
	Base::init();
}

GMZipGamePackageHandler::~GMZipGamePackageHandler()
{
	releaseUnzFile();
}

bool GMZipGamePackageHandler::readFileFromPath(const GMString& path, REF GMBuffer* buffer)
{
	GMString fileName = fromRelativePath(path).toStdString();
	if (loadBuffer(fileName, buffer))
		return true;

	gm_warning(gm_dbg_wrap("cannot find path {0} "), GMString(path));
	return false;
}

void GMZipGamePackageHandler::initFiles()
{
	PKD(d);
	releaseUnzFile();
	m_ufs.clear();

	// 遍历pk0, pk1, ..., pkn，寻找n
	GMsize_t idx = d->packagePath.findLastOf('.');
	if (idx > 0)
	{
		GMString nameWithoutAffix = d->packagePath.substr(0, idx);
		m_packageCount = 1;
		while (true)
		{
			if (!GMPath::fileExists(nameWithoutAffix + ".pk" + GMString(m_packageCount)))
				break;

			++m_packageCount;
		}
	}

	m_ufs.resize(m_packageCount);
}

bool GMZipGamePackageHandler::loadZip()
{
	PKD(d);
	initFiles();

	for (GMint32 n = 0; n < m_packageCount; ++n)
	{
		// 打开zip
		GMsize_t idx = d->packagePath.findLastOf('.');
		GM_ASSERT(idx > 0);
		GMString nameWithoutAffix = d->packagePath.substr(0, idx);
		GMString packagePath = nameWithoutAffix + ".pk" + GMString(n);
		m_ufs[n] = unzOpen64(packagePath.toStdString().c_str());

		// 获取文件名
		unz_global_info64 gi;
		GMint32 err = unzGetGlobalInfo64(m_ufs[n], &gi);
		CHECK(err);
		for (GMint32 i = 0; i < gi.number_entry; i++)
		{
			while (true)
			{
				char filename[FILENAME_MAX];
				unz_file_info64 file_info;
				err = unzGetCurrentFileInfo64(m_ufs[n], &file_info, filename, sizeof(filename), NULL, 0, NULL, 0);
				CHECK(err);

				GMString normalized = normalizePath(filename);
				auto& b = m_buffers[normalized];
				b.first = n;
				break;
			}
			if ((i + 1) < gi.number_entry)
			{
				err = unzGoToNextFile(m_ufs[n]);
				CHECK(err);
			}
		}
	}
	return true;
}

void GMZipGamePackageHandler::releaseUnzFile()
{
	for (auto& uf : m_ufs)
	{
		if (uf)
		{
			unzClose(uf);
			uf = nullptr;
		}
	}
}

GMString GMZipGamePackageHandler::fromRelativePath(const GMString& in)
{
	Deque<std::wstring> deque;

	auto pushToStack = [&deque](std::wstring&& str) {
		if (str == L".")
			return;
		if (str == L"..")
		{
			deque.pop_back();
			return;
		}
		deque.push_back(str);
	};

	size_t pos1 = 0, pos2 = 0;
	GMString normalized = normalizePath(in);
	const std::wstring& stdIn = normalized.toStdWString();
	for (; pos2 < stdIn.length(); pos2++)
	{
		if (stdIn[pos2] == L'\\' || stdIn[pos2] == L'/')
		{
			pushToStack(stdIn.substr(pos1, pos2 - pos1));
			pos1 = ++pos2;
		}
	}
	pushToStack(stdIn.substr(pos1, pos2 - pos1));

	GMString result("");
	while (!deque.empty())
	{
		result += deque.front();
		if (deque.size() > 1)
			result += "/";
		deque.pop_front();
	}
	return result;
}

bool GMZipGamePackageHandler::loadBuffer(const GMString& path, REF GMBuffer* buffer)
{
	constexpr GMuint32 bufSize = 4096;

	// 如果已经有数据了，不需要从zip中读取
	GMString normalized = normalizePath(path);
	auto iter = m_buffers.find(normalized);
	if (iter == m_buffers.end())
		return false;

	GMBuffer& targetBuffer = iter->second.second;
	if (targetBuffer.getSize() > 0)
	{
		*buffer = targetBuffer;
		return true;
	}

	// 获取path的索引，取出相应的zip file
	unzFile uf = m_ufs[iter->second.first];
	std::string stdFileName = normalized.toStdString();
	if (UNZ_OK != unzLocateFile(uf, stdFileName.c_str(), false))
		return false;

	char filename[FILENAME_MAX];
	unz_file_info64 file_info;

	GMint32 err = unzGetCurrentFileInfo64(uf, &file_info, filename, sizeof(filename), NULL, 0, NULL, 0);
	CHECK(err);

	err = unzOpenCurrentFilePassword(uf, nullptr);
	CHECK(err);

	// 构建数据
	targetBuffer.resize(file_info.uncompressed_size);
	GMbyte* ptr = targetBuffer.getData();
	do
	{
		err = unzReadCurrentFile(uf, ptr, bufSize);
		if (err < 0)
			return false;
		if (err > 0)
			ptr += err;
	} while (err > 0);

	*buffer = targetBuffer;
	return true;
}

GMString GMZipGamePackageHandler::pathRoot(GMPackageIndex index)
{
	PKD(d);

	switch (index)
	{
	case GMPackageIndex::Root:
		return L"";
	case GMPackageIndex::Maps:
		return L"maps/";
	case GMPackageIndex::Shaders:
		return L"shaders/";
	case GMPackageIndex::Textures:
		return L"textures/";
	case GMPackageIndex::Models:
		return L"models/";
	case GMPackageIndex::Audio:
		return L"audio/";
	case GMPackageIndex::Particle:
		return L"particles/";
	case GMPackageIndex::Scripts:
		return L"scripts/";
	case GMPackageIndex::Fonts:
		return L"fonts/";
	case GMPackageIndex::Effects:
		return L"effects/";
	case GMPackageIndex::Prefetch:
		return L"prefetch/";
	default:
		GM_ASSERT(false);
		break;
	}
	return L"";
}

GMString GMZipGamePackageHandler::pathOf(GMPackageIndex index, const GMString& fileName)
{
	return normalizePath(pathRoot(index) + fileName);
}

bool GMZipGamePackageHandler::exists(GMPackageIndex index, const GMString& fileName)
{
	GMString path = pathOf(index, normalizePath(fileName));
	bool existed = (m_buffers.find(path) != m_buffers.end());
	if (!fileName.endsWith(L'/') && !existed)
	{
		// 可能它是个目录
		path = pathOf(index, fileName + L"/");
		existed = (m_buffers.find(path) != m_buffers.end());
	}
	return existed;
}

END_NS