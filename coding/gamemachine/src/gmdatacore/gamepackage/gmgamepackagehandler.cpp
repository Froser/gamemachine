#include "stdafx.h"
#include <fstream>
#include "gmgamepackagehandler.h"
#include "foundation/utilities/utilities.h"
#include "extensions/bsp/render/gmbspgameworld.h"
#include "foundation/gamemachine.h"

#define PKD(d) GMGamePackage::Data* d = gamePackage()->gamePackageData();

class GMGLDefaultGamePackageHandler::GMReadFileThread : public GMThread, public IThreadCallback
{
public:
	GMReadFileThread(GMGLDefaultGamePackageHandler* handler, const GMString& path, GMGamePackageAsyncResult* result, GMAsyncCallback callback)
		: m_handler(handler)
		, m_path(path)
		, m_result(result)
		, m_callback(callback)
	{
		setCallback(this);
	}

	virtual void run() override
	{
		GMMutex m;
		GMBuffer* buf = gmobject_cast<GMBuffer*>(m_result->state());
		m_handler->readFileFromPath(m_path, buf);
	}

	virtual void onCreateThread(GMThread*) override
	{
	}

	virtual void beforeRun(GMThread*) override
	{
	}

	virtual void afterRun(GMThread* t) override
	{
		m_callback(m_result);
	}

private:
	GMGLDefaultGamePackageHandler* m_handler;
	GMGamePackageAsyncResult* m_result;
	GMAsyncCallback m_callback;
	GMString m_path;
};

GMGLDefaultGamePackageHandler::GMGLDefaultGamePackageHandler(GMGamePackage* pk)
	: m_pk(pk)
{

}

bool GMGLDefaultGamePackageHandler::readFileFromPath(const GMString& path, REF GMBuffer* buffer)
{
	std::ifstream file;
	std::string p = path.toStdString();
	file.open(p, std::ios::in | std::ios::binary | std::ios::ate);
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

void GMGLDefaultGamePackageHandler::beginReadFileFromPath(const GMString& path, GMAsyncCallback& callback, OUT IAsyncResult** ar)
{
	GMGamePackageAsyncResult* result = new GMGamePackageAsyncResult();
	GMReadFileThread* thread = new GMReadFileThread(this, path, result, callback);
	result->setThread(thread);
	(*ar) = result;
	thread->start();
}

void GMGLDefaultGamePackageHandler::init()
{
}

GMString GMGLDefaultGamePackageHandler::pathRoot(GMPackageIndex index)
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
	case GMPackageIndex::Audio:
		return d->packagePath + "audio/";
	case GMPackageIndex::Scripts:
		return d->packagePath + "scripts/";
	default:
		GM_ASSERT(false);
		break;
	}
	return "";
}

GMGamePackage* GMGLDefaultGamePackageHandler::gamePackage()
{
	return m_pk;
}

Vector<GMString> GMGLDefaultGamePackageHandler::getAllFiles(const GMString& directory)
{
	return GMPath::getAllFiles(directory);
}

#define CHECK(err) if (err != UNZ_OK) return false

GMGLZipGamePackageHandler::GMGLZipGamePackageHandler(GMGamePackage* pk)
	: GMGLDefaultGamePackageHandler(pk)
	, m_uf(nullptr)
{
}

void GMGLZipGamePackageHandler::init()
{
	PKD(d);
	if (!loadZip())
	{
		gm_error(_L("invalid package file %Ls"), d->packagePath.toStdWString().c_str());
		return;
	}
	Base::init();
}

GMGLZipGamePackageHandler::~GMGLZipGamePackageHandler()
{
	releaseUnzFile();
	releaseBuffers();
}

bool GMGLZipGamePackageHandler::readFileFromPath(const GMString& path, REF GMBuffer* buffer)
{
	if (m_buffers.find(path) != m_buffers.end())
	{
		ZipBuffer* buf = m_buffers[path];
		buffer->needRelease = false;
		buffer->buffer = buf->buffer;
		buffer->size = buf->size;
		return true;
	}

	GMString r = toRelativePath(path);
	if (m_buffers.find(toRelativePath(r)) != m_buffers.end())
	{
		ZipBuffer* buf = m_buffers[r];
		buffer->needRelease = false;
		buffer->buffer = buf->buffer;
		buffer->size = buf->size;
		return true;
	}

	gm_warning(_L("cannot find path %Ls"), path.toStdWString().c_str());
	return false;
}

bool GMGLZipGamePackageHandler::loadZip()
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
			GM_ASSERT(m_buffers.find(filename) == m_buffers.end());

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

void GMGLZipGamePackageHandler::releaseUnzFile()
{
	if (m_uf)
	{
		unzClose(m_uf);
		m_uf = nullptr;
	}
}

void GMGLZipGamePackageHandler::releaseBuffers()
{
	for (auto iter = m_buffers.begin(); iter != m_buffers.end(); iter++)
	{
		delete (*iter).second;
	}
}

GMString GMGLZipGamePackageHandler::toRelativePath(const GMString& in)
{
	Deque<GMString> deque;

	auto pushToStack = [&deque](GMString&& str) {
		if (str == ".")
			return;
		if (str == "..")
		{
			deque.pop_back();
			return;
		}
		deque.push_back(str);
	};

	size_t pos1 = 0, pos2 = 0;
	for (; in[pos2]; pos2++)
	{
		if (in[pos2] == '\\' || in[pos2] == '/')
		{
			pushToStack(in.substr(pos1, pos2 - pos1));
			pos1 = ++pos2;
		}
	}
	pushToStack(in.substr(pos1, pos2 - pos1));

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

Vector<GMString> GMGLZipGamePackageHandler::getAllFiles(const GMString& directory)
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

GMString GMGLZipGamePackageHandler::pathRoot(GMPackageIndex index)
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
	case GMPackageIndex::Audio:
		return "audio/";
	case GMPackageIndex::Scripts:
		return "scripts/";
	default:
		GM_ASSERT(false);
		break;
	}
	return "";
}
