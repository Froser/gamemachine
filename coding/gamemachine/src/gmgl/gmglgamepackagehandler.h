#ifndef __GMGLGAMEPACKAGEHANDLER_H__
#define __GMGLGAMEPACKAGEHANDLER_H__
#include "common.h"
#include "gmdatacore/gamepackage.h"
BEGIN_NS

class DefaultGMGLGamePackageHandler : public IGamePackageHandler
{
public:
	DefaultGMGLGamePackageHandler(GamePackage* pk);

public:
	virtual void init() override;
	virtual void readFileFromPath(const char* path, OUT GMbyte** buffer) override;
	std::string pathRoot(PackageIndex index);

private:
	GamePackage* m_pk;
};

END_NS
#endif