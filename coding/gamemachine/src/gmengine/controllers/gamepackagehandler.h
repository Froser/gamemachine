#ifndef __GMGAMEPACKAGEHANDLER_H__
#define __GMGAMEPACKAGEHANDLER_H__
#include "common.h"
#include "gmdatacore/gamepackage.h"
#include "contrib/minizip/unzip.h"
#include <map>
BEGIN_NS

class DefaultGMGamePackageHandler : public IGamePackageHandler
{
public:
	DefaultGMGamePackageHandler(GamePackage* pk);

public:
	virtual void init() override;
	virtual bool readFileFromPath(const char* path, REF GamePackageBuffer* buffer) override;
	virtual std::string pathRoot(PackageIndex index) override;
	virtual AlignedVector<std::string> getAllFiles(const char* directory) override;

protected:
	GamePackage* gamePackage();

private:
	GamePackage* m_pk;
};

class ZipGMGamePackageHandler : public DefaultGMGamePackageHandler
{
	typedef DefaultGMGamePackageHandler Base;

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
	ZipGMGamePackageHandler(GamePackage* pk);
	~ZipGMGamePackageHandler();

public:
	virtual void init() override;
	virtual bool readFileFromPath(const char* path, REF GamePackageBuffer* buffer) override;
	virtual std::string pathRoot(PackageIndex index) override;
	virtual AlignedVector<std::string> getAllFiles(const char* directory) override;

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