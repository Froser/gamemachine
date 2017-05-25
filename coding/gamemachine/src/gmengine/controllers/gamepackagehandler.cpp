#include "stdafx.h"
#include "gmengine/controllers/gamepackagehandler.h"
#include "gmgl/gmglshaders.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmengine/controllers/gamemachine.h"
#include <string>
#include "foundation/utilities/utilities.h"
#include <fstream>
#include "gmengine/elements/bspgameworld.h"

#define PKD(d) GamePackageData* d = gamePackage()->gamePackageData();

DefaultGMGamePackageHandler::DefaultGMGamePackageHandler(GamePackage* pk)
	: m_pk(pk)
{

}

bool DefaultGMGamePackageHandler::readFileFromPath(const char* path, REF GamePackageBuffer* buffer)
{
	std::ifstream file;
	file.open(path, std::ios::in | std::ios::binary | std::ios::ate);
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
		gm_warning("cannot read file from path: %s", path);
	}
	return false;
}

void DefaultGMGamePackageHandler::init()
{
	PKD(d);
	GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(d->gameMachine->getGraphicEngine());

	// 装载所有shaders
	const std::string shaderMap[] = 
	{
		"object",
		"sky"
	};

	for (GMint i = ChildObject::ObjectTypeBegin; i < ChildObject::ObjectTypeEnd; i++)
	{
		GMGLShaders* shaders = new GMGLShaders();
		
		GamePackageBuffer vertBuf, fragBuf;
		gamePackage()->readFile(PI_SHADERS, (shaderMap[i] + ".vert").c_str(), &vertBuf);
		gamePackage()->readFile(PI_SHADERS, (shaderMap[i] + ".frag").c_str(), &fragBuf);
		vertBuf.convertToStringBuffer();
		fragBuf.convertToStringBuffer();

		GMGLShaderInfo shadersInfo[] = {
			{ GL_VERTEX_SHADER, (const char*)vertBuf.buffer },
			{ GL_FRAGMENT_SHADER, (const char*)fragBuf.buffer },
		};

		shaders->appendShader(shadersInfo[0]);
		shaders->appendShader(shadersInfo[1]);
		shaders->load();
		engine->registerShader((ChildObject::ObjectType)i, shaders);
	}
}

std::string DefaultGMGamePackageHandler::pathRoot(PackageIndex index)
{
	PKD(d);

	switch (index)
	{
	case PI_MAPS:
		return d->packagePath + "maps/";
	case PI_SHADERS:
		return d->packagePath + "shaders/";
	case PI_TEXSHADERS:
		return d->packagePath + "texshaders/";
	case PI_TEXTURES:
		return d->packagePath + "textures/";
	default:
		ASSERT(false);
		break;
	}
	return "";
}

GamePackage* DefaultGMGamePackageHandler::gamePackage()
{
	return m_pk;
}

AlignedVector<std::string> DefaultGMGamePackageHandler::getAllFiles(const char* directory)
{
	return Path::getAllFiles(directory);
}

#define CHECK(err) if (err != UNZ_OK) return false

ZipGMGamePackageHandler::ZipGMGamePackageHandler(GamePackage* pk)
	: DefaultGMGamePackageHandler(pk)
	, m_uf(nullptr)
{
}

ZipGMGamePackageHandler::~ZipGMGamePackageHandler()
{
	releaseUnzFile();
	releaseBuffers();
}

void ZipGMGamePackageHandler::init()
{
	PKD(d);
	if (!loadZip())
	{
		gm_error("invalid package file %s", d->packagePath.c_str());
		return;
	}
	Base::init();
}

bool ZipGMGamePackageHandler::readFileFromPath(const char* path, REF GamePackageBuffer* buffer)
{
	if (m_buffers.find(path) != m_buffers.end())
	{
		ZipBuffer* buf = m_buffers[path];
		buffer->needRelease = false;
		buffer->buffer = buf->buffer;
		buffer->size = buf->size;
		return true;
	}
	return false;
}

bool ZipGMGamePackageHandler::loadZip()
{
	const GMuint bufSize = 4096;

	PKD(d);
	releaseUnzFile();

	m_uf = unzOpen64(d->packagePath.c_str());
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

#ifdef _WINDOWS
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

void ZipGMGamePackageHandler::releaseUnzFile()
{
	if (m_uf)
	{
		unzClose(m_uf);
		m_uf = nullptr;
	}
}

void ZipGMGamePackageHandler::releaseBuffers()
{
	for (auto iter = m_buffers.begin(); iter != m_buffers.end(); iter++)
	{
		delete (*iter).second;
	}
}

AlignedVector<std::string> ZipGMGamePackageHandler::getAllFiles(const char* directory)
{
	AlignedVector<std::string> result;
	std::string dir = directory;
	for (auto iter = m_buffers.begin(); iter != m_buffers.end(); iter++)
	{
		if ((*iter).first.compare(0, dir.size(), dir) == 0)
		{
			result.push_back((*iter).first);
		}
	}
	return result;
}

std::string ZipGMGamePackageHandler::pathRoot(PackageIndex index)
{
	PKD(d);

	switch (index)
	{
	case PI_MAPS:
		return "maps/";
	case PI_SHADERS:
		return "shaders/";
	case PI_TEXSHADERS:
		return "texshaders/";
	case PI_TEXTURES:
		return "textures/";
	default:
		ASSERT(false);
		break;
	}
	return "";
}