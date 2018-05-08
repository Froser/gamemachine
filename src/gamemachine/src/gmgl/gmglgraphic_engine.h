#ifndef __GMGLGRAPHIC_ENGINE_H__
#define __GMGLGRAPHIC_ENGINE_H__
#include <gmcommon.h>
#include <gmassets.h>
#include "gmglshaderprogram.h"
#include <gamemachine.h>
#include <map>
#include <gmgraphicengine.h>
#include "gmglgbuffer.h"
BEGIN_NS

class Camera;
class GMGameWorld;
class GameLight;
struct IRenderer;

extern "C"
{
	extern GMuint s_glErrCode;
}

enum
{
	DEFERRED_GEOMETRY_PASS_SHADER,
	DEFERRED_LIGHT_PASS_SHADER,
};

struct GMGLLightContext
{
	bool lightDirty = true;
	IShaderProgram* shaderProgram = nullptr;
};

GM_PRIVATE_OBJECT(GMGLGraphicEngine)
{
	bool engineReady = false;

	// 著色器程序
	GMGLShaderProgram* forwardShaderProgram = nullptr;
	GMGLShaderProgram* deferredShaderPrograms[2] = { nullptr };
	GMGLShaderProgram* filterShaderProgram = nullptr;

	ITexture* cubeMap = nullptr;
	GMGLLightContext lightContext;

	// 混合绘制
	GMS_BlendFunc blendsfactor;
	GMS_BlendFunc blenddfactor;
	bool isBlending = false;
};

class GMGLGraphicEngine : public GMGraphicEngine
{
	DECLARE_PRIVATE_AND_BASE(GMGLGraphicEngine, GMGraphicEngine)

public:
	~GMGLGraphicEngine();

public:
	virtual void init() override;
	virtual void update(GMUpdateDataType type) override;
	virtual void clearStencil() override;
	virtual void beginBlend(GMS_BlendFunc sfactor, GMS_BlendFunc dfactor) override;
	virtual void endBlend() override;
	virtual IShaderProgram* getShaderProgram(GMShaderProgramType type) override;
	virtual bool event(const GameMachineMessage& e) override { return false; }
	virtual IFramebuffers* getDefaultFramebuffers() override;

public:
	virtual bool getInterface(GameMachineInterfaceID, void**) { return false; }
	virtual bool setInterface(GameMachineInterfaceID, void*);
	virtual void createShadowFramebuffers(OUT IFramebuffers** framebuffers) override;

public:
	IRenderer* getRenderer(GMModelType objectType);

public:
	inline bool isBlending() { D(d); return d->isBlending; }
	GMS_BlendFunc blendsfactor() { D(d); return d->blendsfactor; }
	GMS_BlendFunc blenddfactor() { D(d); return d->blenddfactor; }

	inline void setCubeMap(ITexture* tex)
	{
		D(d);
		if (d->cubeMap != tex)
			d->cubeMap = tex;
	}

	inline ITexture* getCubeMap()
	{
		D(d);
		return d->cubeMap;
	}

public:
	void activateLights(IRenderer* renderer);

private:
	void installShaders();
};

class GMGLUtility
{
public:
	static void blendFunc(GMS_BlendFunc sfactor, GMS_BlendFunc dfactor);
};

END_NS
#endif