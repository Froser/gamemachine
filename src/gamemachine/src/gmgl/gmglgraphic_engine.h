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

	GMGLGBufferDep gbuffer;
	ITexture* cubeMap = nullptr;

	// 延迟渲染的四边形
	GMuint quadVAO = 0;
	GMuint quadVBO = 0;
	// 渲染状态
	GMGLDeferredRenderState renderState = GMGLDeferredRenderState::PassingGeometry;

	// 延迟渲染分组
	Vector<GMGameObject*> deferredRenderingGameObjects;
	Vector<GMGameObject*> forwardRenderingGameObjects;

	GMRenderMode renderMode = GMRenderMode::Forward;
	GMStencilOptions stencilOptions;

	// 混合绘制
	GMRenderMode renderModeForBlend = GMRenderMode::Forward;
	GMS_BlendFunc blendsfactor;
	GMS_BlendFunc blenddfactor;
	bool isBlending = false;

	// 是否进入绘制模式
#if _DEBUG
	GMint drawingLevel = 0;
#endif
};

class GMGLGraphicEngine : public GMGraphicEngine
{
	DECLARE_PRIVATE(GMGLGraphicEngine)

public:
	GMGLGraphicEngine();
	~GMGLGraphicEngine();

public:
	virtual void init() override;
	virtual void newFrame() override;
	virtual bool event(const GameMachineMessage& e) override;
	virtual void drawObjects(GMGameObject *objects[], GMuint count) override;
	virtual void update(GMUpdateDataType type) override;
	virtual void addLight(const GMLight& light) override;
	virtual void removeLights();
	virtual void clearStencil() override;
	virtual void beginBlend(GMS_BlendFunc sfactor, GMS_BlendFunc dfactor) override;
	virtual void endBlend() override;
	virtual IShaderProgram* getShaderProgram(GMShaderProgramType type) override;
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
	void updateShader();

public:
	inline GMGLDeferredRenderState getRenderState() { D(d); return d->renderState; }
	inline bool isBlending() { D(d); return d->isBlending; }
	GMS_BlendFunc blendsfactor() { D(d); return d->blendsfactor; }
	GMS_BlendFunc blenddfactor() { D(d); return d->blenddfactor; }
	GMRenderMode getCurrentRenderMode() { D(d); return d->renderMode; }

	inline void setRenderState(GMGLDeferredRenderState state)
	{
		D(d);
		d->renderState = state; 
		updateShader();
	}

	inline void setCurrentRenderMode(GMRenderMode mode)
	{
		D(d);
		d->renderMode = mode;
		updateShader();
	}

	inline void checkDrawingState()
	{
#if _DEBUG
		D(d);
		if (!d->drawingLevel)
		{
			GM_ASSERT(false);
			gm_error("GMObject::draw() cannot be called outside IGraphicEngine::drawObjects");
		}
#endif
	}

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

private:
	void directDraw(GMGameObject *objects[], GMuint count);
	void forwardDraw(GMGameObject *objects[], GMuint count);
	void activateLightsIfNecessary();
	void createDeferredRenderQuad();
	void renderDeferredRenderQuad();
	void disposeDeferredRenderQuad();
	void updateProjectionMatrix();
	void updateViewMatrix();
	void installShaders();
	void activateLights(const Vector<GMLight>& lights);
	bool refreshGBuffer();
	void forwardRender(GMGameObject* objects[], GMuint count);
	void geometryPass(Vector<GMGameObject*>& objects);
	void lightPass();
	void groupGameObjects(GMGameObject *objects[], GMuint count);
	void viewGBufferFrameBuffer();

public:
	static void newFrameOnCurrentFramebuffer();
	static void clearStencilOnCurrentFramebuffer();
};

class GMGLUtility
{
public:
	static void blendFunc(GMS_BlendFunc sfactor, GMS_BlendFunc dfactor);
};

END_NS
#endif