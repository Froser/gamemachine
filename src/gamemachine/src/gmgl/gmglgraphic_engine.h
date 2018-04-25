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

GM_PRIVATE_OBJECT(GMGLGraphicEngine)
{
	bool needRefreshLights = true;
	Vector<GMLight> lights;
	bool engineReady = false;
	GMRenderConfig renderConfig;
	GMDebugConfig debugConfig;
	// 著色器程序
	GMGLShaderProgram* forwardShaderProgram = nullptr;
	GMGLShaderProgram* deferredShaderPrograms[2] = { nullptr };
	GMGLShaderProgram* filterShaderProgram = nullptr;
	IShaderLoadCallback* shaderLoadCallback = nullptr;
	GraphicSettings* settings = nullptr;

	IGBuffer* gbuffer = nullptr;
	ITexture* cubeMap = nullptr;

	// 延迟渲染分组
	Vector<GMGameObject*> deferredRenderingGameObjects;
	Vector<GMGameObject*> forwardRenderingGameObjects;

	GMStencilOptions stencilOptions;

	// 混合绘制
	GMS_BlendFunc blendsfactor;
	GMS_BlendFunc blenddfactor;
	bool isBlending = false;
};

class GMGLGraphicEngine : public GMGraphicEngine
{
	DECLARE_PRIVATE_AND_BASE(GMGLGraphicEngine, GMGraphicEngine)

public:
	GMGLGraphicEngine();
	~GMGLGraphicEngine();

public:
	virtual void init() override;
	virtual void newFrame() override;
	virtual void drawObjects(GMGameObject *objects[], GMuint count) override;
	virtual void update(GMUpdateDataType type) override;
	virtual void addLight(const GMLight& light) override;
	virtual void removeLights();
	virtual void clearStencil() override;
	virtual void beginBlend(GMS_BlendFunc sfactor, GMS_BlendFunc dfactor) override;
	virtual void endBlend() override;
	virtual IShaderProgram* getShaderProgram(GMShaderProgramType type) override;
	virtual bool event(const GameMachineMessage& e) override { return false; }

	virtual void setShaderLoadCallback(IShaderLoadCallback* cb) override
	{
		D(d);
		d->shaderLoadCallback = cb;
	}

	virtual void setStencilOptions(const GMStencilOptions& options) override
	{
		D(d);
		d->stencilOptions = options;
	}

	virtual const GMStencilOptions& getStencilOptions() override
	{
		D(d);
		return d->stencilOptions;
	}

	const GMFilterMode::Mode getCurrentFilterMode()
	{
		D(d);
		return d->renderConfig.get(GMRenderConfigs::FilterMode).toEnum<GMFilterMode::Mode>();
	}

public:
	virtual bool getInterface(GameMachineInterfaceID, void**) { return false; }
	virtual bool setInterface(GameMachineInterfaceID, void*);

public:
	IRenderer* getRenderer(GMModelType objectType);
	void setViewport(const GMRect& rect);

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
	void draw(GMGameObject* objects[], GMuint count);
	void activateLights(IShaderProgram* shaderProgram);

private:
	void directDraw(GMGameObject *objects[], GMuint count);
	void forwardDraw(GMGameObject *objects[], GMuint count);
	void activateLightsIfNecessary();
	void updateProjectionMatrix();
	void updateViewMatrix();
	void installShaders();
	void groupGameObjects(GMGameObject *objects[], GMuint count);

public:
	static void clearStencilOnCurrentFramebuffer();
};

class GMGLUtility
{
public:
	static void blendFunc(GMS_BlendFunc sfactor, GMS_BlendFunc dfactor);
};

END_NS
#endif