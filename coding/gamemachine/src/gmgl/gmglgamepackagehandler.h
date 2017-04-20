#ifndef __GMGLGAMEPACKAGEHANDLER_H__
#define __GMGLGAMEPACKAGEHANDLER_H__
#include "common.h"
#include "gmdatacore/gamepackage.h"
BEGIN_NS

enum GMGLGamePackageFileType
{
	GMPF_SHADERS, //GLSL着色器
};

class DefaultGMGLGamePackageHandler : public IGamePackageHandler
{
public:
	DefaultGMGLGamePackageHandler(GamePackage* pk);

public:
	virtual void init() override;
	virtual GMbyte* readFileFromPath(const char* path) override;

private:
	virtual GMbyte* readFileFromPath(GMGLGamePackageFileType type, const char* path);

private:
	GamePackage* m_pk;
};

END_NS
#endif