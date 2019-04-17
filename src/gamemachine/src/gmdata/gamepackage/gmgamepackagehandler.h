#ifndef __GMGAMEPACKAGEHANDLER_H__
#define __GMGAMEPACKAGEHANDLER_H__
#include <gmcommon.h>
#include <gmthread.h>
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
	virtual GMString pathOf(GMPackageIndex index, const GMString& fileName) override;
	virtual bool exists(GMPackageIndex index, const GMString& fileName) override;

protected:
	virtual GMString pathRoot(GMPackageIndex index);

protected:
	void resetPackageCandidate();
	bool nextPackageCandidate();
	GMGamePackage* gamePackage();
	GMString packagePath();

private:
	GMGamePackage* m_pk;

protected:
	GMint32 m_packageIndex;
	GMMutex m_mutex;
};

class GMZipGamePackageHandler : public GMDefaultGamePackageHandler
{
	typedef GMDefaultGamePackageHandler Base;

public:
	GMZipGamePackageHandler(GMGamePackage* pk);
	~GMZipGamePackageHandler();

public:
	virtual void init() override;
	virtual bool readFileFromPath(const GMString& path, REF GMBuffer* buffer) override;
	virtual GMString pathOf(GMPackageIndex index, const GMString& fileName) override;
	virtual bool exists(GMPackageIndex index, const GMString& fileName) override;

protected:
	virtual GMString pathRoot(GMPackageIndex index) override;

private:
	void initFiles();
	bool loadZip();
	void releaseUnzFile();
	void releaseBuffers();
	GMString fromRelativePath(const GMString& in);
	bool loadBuffer(const GMString& path, REF GMBuffer* buffer);

private:
	Vector<unzFile> m_ufs;
	HashMap<GMString, Pair<GMint32, GMBuffer*>, GMStringHashFunctor> m_buffers;
	GMint32 m_packageCount;

protected:
	GMint32 m_packageIndex;
};

END_NS
#endif