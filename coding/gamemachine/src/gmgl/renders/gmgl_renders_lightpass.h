#ifndef __GMGL_RENDERS_LIGHTPASS_H__
#define __GMGL_RENDERS_LIGHTPASS_H__
#include <gmcommon.h>
#include <gmmodel.h>
BEGIN_NS

class GMGLGraphicEngine;
GM_PRIVATE_OBJECT(GMGLRenders_LightPass)
{
	GMGLGraphicEngine* engine;
};

class GMShader;
class GMGLRenders_LightPass : public GMObject, public IRender
{
	DECLARE_PRIVATE(GMGLRenders_LightPass)

public:
	GMGLRenders_LightPass();

public:
	virtual void begin(GMMesh* mesh, const GMGameObject* parent) override {}
	virtual void beginShader(GMShader& shader) override { GM_ASSERT(false); }
	virtual void endShader() { GM_ASSERT(false); }
	virtual void end() {}
};

END_NS
#endif