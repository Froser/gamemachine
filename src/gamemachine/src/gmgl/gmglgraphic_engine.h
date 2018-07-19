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
	GMOwnedPtr<GMGLShaderProgram> forwardShaderProgram;
	GMOwnedPtr<GMGLShaderProgram> deferredShaderPrograms[2];
	GMOwnedPtr<GMGLShaderProgram> filterShaderProgram;

	// 渲染器
	GMOwnedPtr<IRenderer> renderer_2d;
	GMOwnedPtr<IRenderer> renderer_3d;
	GMOwnedPtr<IRenderer> renderer_cubeMap;
	GMOwnedPtr<IRenderer> renderer_filter;
	GMOwnedPtr<IRenderer> renderer_lightPass;
	GMOwnedPtr<IRenderer> renderer_3d_shadow;
	GMOwnedPtr<IRenderer> renderer_particle;

	ITexture* cubeMap = nullptr;
	GMGLLightContext lightContext;
};

class GMGLGraphicEngine : public GMGraphicEngine
{
	GM_DECLARE_PRIVATE_AND_BASE(GMGLGraphicEngine, GMGraphicEngine)

public:
	GMGLGraphicEngine(const IRenderContext* context);
	~GMGLGraphicEngine() = default;

public:
	virtual void init() override;
	virtual void update(GMUpdateDataType type) override;
	virtual IShaderProgram* getShaderProgram(GMShaderProgramType type) override;
	virtual bool event(const GMMessage& e) override { return false; }
	virtual IFramebuffers* getDefaultFramebuffers() override;
	virtual IRenderer* getRenderer(GMModelType objectType) override;
	virtual GMGlyphManager* getGlyphManager() override;

public:
	virtual bool getInterface(GameMachineInterfaceID, void**) { return false; }
	virtual bool setInterface(GameMachineInterfaceID, void*);
	virtual void createShadowFramebuffers(OUT IFramebuffers** framebuffers) override;

public:
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
	void shaderProgramChanged(IShaderProgram* program);

private:
	void installShaders();
};

class GMGLUtility
{
public:
	static void blendFunc(
		GMS_BlendFunc sfactorRGB,
		GMS_BlendFunc dfactorRGB,
		GMS_BlendOp opRGB,
		GMS_BlendFunc sfactorAlpha,
		GMS_BlendFunc dfactorAlpha,
		GMS_BlendOp opAlpha
	);
};

END_NS
#endif