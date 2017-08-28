﻿#ifndef __GMGLGAMEPACKAGEHANDLER_H__
#define __GMGLGAMEPACKAGEHANDLER_H__
#include "common.h"
#include "gmdatacore/gamepackage/gmgamepackage.h"
#include "contrib/minizip/unzip.h"
#include <map>
BEGIN_NS

class GMGLDefaultGamePackageHandler : public IGamePackageHandler
{
public:
	GMGLDefaultGamePackageHandler(GMGamePackage* pk);

public:
	virtual void init() override;
	virtual bool readFileFromPath(const GMString& path, REF GMBuffer* buffer) override;
	virtual GMString pathRoot(GMPackageIndex index) override;
	virtual Vector<GMString> getAllFiles(const GMString& directory) override;

protected:
	GMGamePackage* gamePackage();

private:
	GMGamePackage* m_pk;
};

class GMGLZipGamePackageHandler : public GMGLDefaultGamePackageHandler
{
	typedef GMGLDefaultGamePackageHandler Base;

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
	GMGLZipGamePackageHandler(GMGamePackage* pk);
	~GMGLZipGamePackageHandler();

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