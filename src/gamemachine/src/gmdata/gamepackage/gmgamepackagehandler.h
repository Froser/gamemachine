#ifndef __GMGAMEPACKAGEHANDLER_H__
#define __GMGAMEPACKAGEHANDLER_H__
#include <gmcommon.h>
#include "gmgamepackage.h"
#include "contrib/minizip/unzip.h"
BEGIN_NS

class GMDefaultGamePackageHandler : public IGamePackageHandler
{
public:
	GMDefaultGamePackageHandler(GMGamePackage* pk);

public:
	virtual void init() override;
	virtual bool readFileFromPath(const GMString& path, REF GMBuffer* buffer) override;
	virtual void beginReadFileFromPath(const GMString& path, GMAsyncCallback callback, OUT GMAsyncResult** ar) override;
	virtual GMString pathRoot(GMPackageIndex index) override;
	virtual Vector<GMString> getAllFiles(const GMString& directory) override;

protected:
	GMGamePackage* gamePackage();

protected:
	class GMReadFileThread;

private:
	GMGamePackage* m_pk;
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

		GMuint32 size;
		GMbyte* buffer;
	};

public:
	GMZipGamePackageHandler(GMGamePackage* pk);
	~GMZipGamePackageHandler();

public:
	virtual void init() override;
	virtual bool readFileFromPath(const GMString& path, REF GMBuffer* buffer) override;
	virtual GMString pathRoot(GMPackageIndex index) override;
	virtual Vector<GMString> getAllFiles(const GMString& directory) override;

private:
	bool loadZip();
	void releaseUnzFile();
	void releaseBuffers();
	GMString toRelativePath(const GMString& in);

private:
	unzFile m_uf;
	Map<GMString, ZipBuffer*> m_buffers;
};

END_NS
#endif