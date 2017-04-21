#include "stdafx.h"
#include "gmglgamepackagehandler.h"
#include "gmglshaders.h"
#include "gmglgraphic_engine.h"
#include "gmengine/controllers/gamemachine.h"
#include <string>
#include "utilities/path.h"
#include <fstream>
#include "gmengine/elements/bspgameworld.h"

#define PKD(d) GamePackageData& d = gamePackage()->gamePackageData();

DefaultGMGLGamePackageHandler::DefaultGMGLGamePackageHandler(GamePackage* pk)
	: m_pk(pk)
{

}

void DefaultGMGLGamePackageHandler::readFileFromPath(const char* path, REF GamePackageBuffer* buffer)
{

	std::ifstream file;
	file.open(path, std::ios::in | std::ios::binary);
	if (file.good())
	{
		file.seekg(0, std::ios::end);
		GMint size = file.tellg();
		buffer->buffer = new GMbyte(size + 1);
		file.seekg(0);
		file.read((char*)(buffer->buffer), size + 1);
		buffer->needRelease = true;
	}
	else
	{
		ASSERT(false);
	}
}

void DefaultGMGLGamePackageHandler::init()
{
	PKD(d);
	GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(d.gameMachine->getGraphicEngine());

	// 装载所有shaders
	const std::string shaderMap[] = 
	{
		"object",
		"sky"
	};

	for (GMint i = ChildObject::ObjectTypeBegin; i < ChildObject::ObjectTypeEnd; i++)
	{
		GMGLShaders* shaders = new GMGLShaders();
		
		std::string vert = gamePackage()->path(PI_SHADERS, (shaderMap[i] + ".vert").c_str()),
			frag = gamePackage()->path(PI_SHADERS, (shaderMap[i] + ".frag").c_str());

		GamePackageBuffer vertBuf, fragBuf;
		readFileFromPath(vert.c_str(), &vertBuf);
		readFileFromPath(frag.c_str(), &fragBuf);

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

std::string DefaultGMGLGamePackageHandler::pathRoot(PackageIndex index)
{
	PKD(d);

	switch (index)
	{
	case PI_MAPS:
		return d.packagePath + "maps/";
	case PI_SHADERS:
		return d.packagePath + "shaders/";
	case PI_TEXSHADERS:
		return d.packagePath + "texshaders/";
	case PI_TEXTURES:
		return d.packagePath + "textures/";
	default:
		ASSERT(false);
		break;
	}
	return "";
}

GamePackage* DefaultGMGLGamePackageHandler::gamePackage()
{
	return m_pk;
}

std::vector<std::string> DefaultGMGLGamePackageHandler::getAllFiles(const char* directory)
{
	return Path::getAllFiles(directory);
}

#define CHECK(err) if (err != UNZ_OK) return false

ZipGMGLGamePackageHandler::ZipGMGLGamePackageHandler(GamePackage* pk)
	: DefaultGMGLGamePackageHandler(pk)
	, m_uf(nullptr)
{
}

ZipGMGLGamePackageHandler::~ZipGMGLGamePackageHandler()
{
	releaseUnzFile();
	releaseBuffers();
}

void ZipGMGLGamePackageHandler::init()
{
	PKD(d);
	if (!loadZip())
	{
		gm_error("invalid package file %s", d.packagePath.c_str());
		return;
	}
	Base::init();
}

void ZipGMGLGamePackageHandler::readFileFromPath(const char* path, REF GamePackageBuffer* buffer)
{
	ASSERT(m_buffers.find(path) != m_buffers.end());
	ZipBuffer* buf = m_buffers[path];
	buffer->needRelease = false;
	buffer->buffer = buf->buffer;
}

bool ZipGMGLGamePackageHandler::loadZip()
{
	const GMuint bufSize = 4096;

	PKD(d);
	releaseUnzFile();

	m_uf = unzOpen64(d.packagePath.c_str());
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

std::vector<std::string> ZipGMGLGamePackageHandler::getAllFiles(const char* directory)
{
	std::vector<std::string> result;
	std::string d = directory;
	for (auto iter = m_buffers.begin(); iter != m_buffers.end(); iter++)
	{
		if ((*iter).first.compare(0, d.size(), d) == 0)
		{
			result.push_back((*iter).first);
		}
	}
	return result;
}

std::string ZipGMGLGamePackageHandler::pathRoot(PackageIndex index)
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