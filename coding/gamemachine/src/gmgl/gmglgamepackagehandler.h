#ifndef __GMGLGAMEPACKAGEHANDLER_H__
#define __GMGLGAMEPACKAGEHANDLER_H__
#include "common.h"
#include "gmdatacore/gamepackage/gmgamepackage.h"
#include "contrib/minizip/unzip.h"
#include <map>
BEGIN_NS

class GMDefaultGLGamePackageHandler : public IGamePackageHandler
{
public:
	GMDefaultGLGamePackageHandler(GMGamePackage* pk);

public:
	virtual void init() override;
	virtual bool readFileFromPath(const char* path, REF GMBuffer* buffer) override;
	virtual std::string pathRoot(PackageIndex index) override;
	virtual AlignedVector<GMString> getAllFiles(const GMString& directory) override;

protected:
	GMGamePackage* gamePackage();

private:
	GMGamePackage* m_pk;
};

class ZipGMGLGamePackageHandler : public GMDefaultGLGamePackageHandler
{
	typedef GMDefaultGLGamePackageHandler Base;

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
	ZipGMGLGamePackageHandler(GMGamePackage* pk);
	~ZipGMGLGamePackageHandler();

public:
	virtual void init() override;
	virtual bool readFileFromPath(const char* path, REF GMBuffer* buffer) override;
	virtual std::string pathRoot(PackageIndex index) override;
	virtual AlignedVector<GMString> getAllFiles(const GMString& directory) override;

private:
	bool loadZip();
	void releaseUnzFile();
	void releaseBuffers();

private:
	unzFile m_uf;
	std::map<GMString, ZipBuffer*> m_buffers;
};

END_NS
#endif