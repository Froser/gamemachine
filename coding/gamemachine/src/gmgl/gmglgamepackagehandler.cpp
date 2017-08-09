#include "stdafx.h"
#include "gmglgamepackagehandler.h"
#include "gmglshaderprogram.h"
#include "gmglgraphic_engine.h"
#include <string>
#include "foundation/utilities/utilities.h"
#include <fstream>
#include "gmengine/gmbspgameworld.h"
#include "renders/gmgl_renders_object.h"
#include "renders/gmgl_renders_glyph.h"
#include "foundation/gamemachine.h"

#define PKD(d) GMGamePackage::Data* d = gamePackage()->gamePackageData();

GMDefaultGLGamePackageHandler::GMDefaultGLGamePackageHandler(GMGamePackage* pk)
	: m_pk(pk)
{

}

bool GMDefaultGLGamePackageHandler::readFileFromPath(const GMString& path, REF GMBuffer* buffer)
{
	std::ifstream file;
	file.open(path.toStdWString(), std::ios::in | std::ios::binary | std::ios::ate);
	if (file.good())
	{
		GMint size = file.tellg();
		buffer->size = size;
		buffer->buffer = new GMbyte[buffer->size];
		buffer->needRelease = true;

		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(buffer->buffer), size);
		file.close();
		return true;
	}
	else
	{
		gm_warning(_L("cannot read file from path: %Ls"), path.toStdWString().c_str());
	}
	return false;
}

void GMDefaultGLGamePackageHandler::init()
{
}

GMString GMDefaultGLGamePackageHandler::pathRoot(GMPackageIndex index)
{
	PKD(d);

	switch (index)
	{
	case GMPackageIndex::Maps:
		return d->packagePath + "maps/";
	case GMPackageIndex::Shaders:
		return d->packagePath + "shaders/";
	case GMPackageIndex::TexShaders:
		return d->packagePath + "texshaders/";
	case GMPackageIndex::Textures:
		return d->packagePath + "textures/";
	case GMPackageIndex::Models:
		return d->packagePath + "models/";
	case GMPackageIndex::Sounds:
		return d->packagePath + "sounds/";
	case GMPackageIndex::Scripts:
		return d->packagePath + "scripts/";
	default:
		ASSERT(false);
		break;
	}
	return "";
}

GMGamePackage* GMDefaultGLGamePackageHandler::gamePackage()
{
	return m_pk;
}

Vector<GMString> GMDefaultGLGamePackageHandler::getAllFiles(const GMString& directory)
{
	return Path::getAllFiles(directory);
}

#define CHECK(err) if (err != UNZ_OK) return false

ZipGMGLGamePackageHandler::ZipGMGLGamePackageHandler(GMGamePackage* pk)
	: GMDefaultGLGamePackageHandler(pk)
	, m_uf(nullptr)
{
}

void ZipGMGLGamePackageHandler::init()
{
	PKD(d);
	if (!loadZip())
	{
		gm_error(_L("invalid package file %Ls"), d->packagePath.toStdWString().c_str());
		return;
	}
	Base::init();
}

ZipGMGLGamePackageHandler::~ZipGMGLGamePackageHandler()
{
	releaseUnzFile();
	releaseBuffers();
}

bool ZipGMGLGamePackageHandler::readFileFromPath(const GMString& path, REF GMBuffer* buffer)
{
	if (m_buffers.find(path) != m_buffers.end())
	{
		ZipBuffer* buf = m_buffers[path];
		buffer->needRelease = false;
		buffer->buffer = buf->buffer;
		buffer->size = buf->size;
		return true;
	}
	gm_warning(_L("cannot find path %Ls"), path.toStdWString().c_str());
	return false;
}

bool ZipGMGLGamePackageHandler::loadZip()
{
	const GMuint bufSize = 4096;

	PKD(d);
	releaseUnzFile();

	m_uf = unzOpen64(d->packagePath.toStdString().c_str());
	unz_global_info64 gi;
	GMint err = unzGetGlobalInfo64(m_uf, &gi);
	CHECK(err);

	for (GMint i = 0; i < gi.number_entry; i++)
	{
		while (true)
		{
			char filename[FILENAME_MAX];
			unz_file_info64 file_info;
			err = unzGetCurrentFileInfo64(m_uf, &file_info, filename, sizeof(filename), NULL, 0, NULL, 0);
			CHECK(err);

			err = unzOpenCurrentFilePassword(m_uf, nullptr);
			CHECK(err);

#if _WINDOWS
			// 跳过文件夹
			if (file_info.external_fa & FILE_ATTRIBUTE_DIRECTORY)
				break;
#endif

			GMbyte* buffer = new GMbyte[file_info.uncompressed_size];
			GMbyte* ptr = buffer;
			do
			{
				err = unzReadCurrentFile(m_uf, ptr, bufSize);
				if (err < 0)
					return false;
				if (err > 0)
					ptr += err;
			} while (err > 0);
			ASSERT(m_buffers.find(filename) == m_buffers.end());

			ZipBuffer* buf = new ZipBuffer();
			buf->buffer = buffer;
			buf->size = file_info.uncompressed_size;
			m_buffers[filename] = buf;

			break;
		}
		if ((i + 1) < gi.number_entry)
		{
			err = unzGoToNextFile(m_uf);
			CHECK(err);
		}
	}

	releaseUnzFile();
	return true;
}

void ZipGMGLGamePackageHandler::releaseUnzFile()
{
	if (m_uf)
	{
		unzClose(m_uf);
		m_uf = nullptr;
	}
}

void ZipGMGLGamePackageHandler::releaseBuffers()
{
	for (auto iter = m_buffers.begin(); iter != m_buffers.end(); iter++)
	{
		delete (*iter).second;
	}
}

Vector<GMString> ZipGMGLGamePackageHandler::getAllFiles(const GMString& directory)
{
	Vector<GMString> result;
	GMString d = directory;
	for (auto& buffer : m_buffers)
	{
		if (buffer.first.toStdWString().compare(0, d.length(), d.toStdWString()) == 0)
		{
			result.push_back(buffer.first);
		}
	}
	return result;
}

GMString ZipGMGLGamePackageHandler::pathRoot(GMPackageIndex index)
{
	PKD(d);

	switch (index)
	{
	case GMPackageIndex::Maps:
		return "maps/";
	case GMPackageIndex::Shaders:
		return "shaders/";
	case GMPackageIndex::TexShaders:
		return "texshaders/";
	case GMPackageIndex::Textures:
		return "textures/";
	case GMPackageIndex::Models:
		return "models/";
	case GMPackageIndex::Sounds:
		return "sounds/";
	case GMPackageIndex::Scripts:
		return "scripts/";
	default:
		ASSERT(false);
		break;
	}
	return "";
}