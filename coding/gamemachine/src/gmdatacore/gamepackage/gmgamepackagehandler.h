#ifndef __GMGAMEPACKAGEHANDLER_H__
#define __GMGAMEPACKAGEHANDLER_H__
#include "common.h"
#include "gmdatacore/gamepackage/gmgamepackage.h"
#include "contrib/minizip/unzip.h"
#include <map>
BEGIN_NS

GM_PRIVATE_OBJECT(GMDefaultGamePackageHandler)
{
	GMGamePackage* gamePackage;
};

class GMDefaultGamePackageHandler : public GMObject, public IGamePackageHandler
{
	DECLARE_PRIVATE(GMDefaultGamePackageHandler)

public:
	GMDefaultGamePackageHandler(GMGamePackage* pk);

public:
	virtual void init() override;
	virtual bool readFileFromPath(const char* path, REF GMBuffer* buffer) override;
	virtual std::string pathRoot(PackageIndex index) override;
	virtual AlignedVector<GMString> getAllFiles(const GMString& directory) override;

protected:
	GMGamePackage* gamePackage();
};

class GMZipGamePackageHandler : public GMDefaultGamePackageHandler
{
	typedef GMDefaultGamePackageHandler Base;

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
	GMZipGamePackageHandler(GMGamePackage* pk);
	~GMZipGamePackageHandler();

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