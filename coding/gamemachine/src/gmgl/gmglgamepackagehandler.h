#ifndef __GMGLGAMEPACKAGEHANDLER_H__
#define __GMGLGAMEPACKAGEHANDLER_H__
#include "common.h"
#include "gmdatacore/gamepackage.h"
#include "contrib/minizip/unzip.h"
#include <map>
BEGIN_NS

class DefaultGMGLGamePackageHandler : public IGamePackageHandler
{
public:
	DefaultGMGLGamePackageHandler(GamePackage* pk);

public:
	virtual void init() override;
	virtual bool readFileFromPath(const char* path, REF GamePackageBuffer* buffer) override;
	virtual std::string pathRoot(PackageIndex index) override;
	virtual Vector<std::string> getAllFiles(const char* directory) override;

protected:
	GamePackage* gamePackage();

private:
	GamePackage* m_pk;
};

class ZipGMGLGamePackageHandler : public DefaultGMGLGamePackageHandler
{
	typedef DefaultGMGLGamePackageHandler Base;

	struct ZipBuffer
	{
		ZipBuffer()
			: buffer(nullptr)
			, size(0)
		{
		}

		~ZipBuffer()
		{
			if (buffer)
				delete buffer;
		}

		GMuint size;
		GMbyte* buffer;
	};

public:
	ZipGMGLGamePackageHandler(GamePackage* pk);
	~ZipGMGLGamePackageHandler();

public:
	virtual void init() override;
	virtual bool readFileFromPath(const char* path, REF GamePackageBuffer* buffer) override;
	virtual std::string pathRoot(PackageIndex index) override;
	virtual Vector<std::string> getAllFiles(const char* directory) override;

private:
	bool loadZip();
	void releaseUnzFile();
	void releaseBuffers();

private:
	unzFile m_uf;
	std::map<std::string, ZipBuffer*> m_buffers;
};

END_NS
#endif