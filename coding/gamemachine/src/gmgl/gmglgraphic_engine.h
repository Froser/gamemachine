#ifndef __GMGLGRAPHIC_ENGINE_H__
#define __GMGLGRAPHIC_ENGINE_H__
#include <gmcommon.h>
#include <gmassets.h>
#include "gmglshaderprogram.h"
#include <gamemachine.h>
#include <map>
#include "gmglgbuffer.h"
BEGIN_NS

class Camera;
class GMGameWorld;
class GameLight;
struct IRender;

extern "C"
{
	extern GLenum s_glErrCode;
}

struct GMShaderProc
{
	enum
	{
		GEOMETRY_PASS,
		MATERIAL_PASS,
	};
};

#define GM_CHECK_GL_ERROR() GM_ASSERT((s_glErrCode = glGetError()) == GL_NO_ERROR);
#define GM_BEGIN_CHECK_GL_ERROR { GM_CHECK_GL_ERROR()
#define GM_END_CHECK_GL_ERROR GM_CHECK_GL_ERROR() }

GM_INTERFACE(IShaderLoadCallback)
{
	virtual void onLoadEffectsShader(GMGLShaderProgram& effectsProgram) = 0;
	virtual void onLoadShaderProgram(GMGLShaderProgram& forwardShaderProgram, GMGLShaderProgram* deferredShaderProgram[2]) = 0;
};

enum
{
	DEFERRED_GEOMETRY_PASS_SHADER,
	DEFERRED_LIGHT_PASS_SHADER,
};

GM_PRIVATE_OBJECT(GMGLGraphicEngine)
{
	bool needRefreshLights = true;
	Vector<GMLight> lights;

	// 著色器程序
	GMGLShaderProgram* forwardShaderProgram = nullptr;
	GMGLShaderProgram* deferredShaderProgram[2] = { nullptr };
	GMGLShaderProgram* effectsShaderProgram = nullptr;
	IShaderLoadCallback* shaderLoadCallback = nullptr;
	GraphicSettings* settings = nullptr;

	GMGLGBuffer gbuffer;
	GMGLFramebuffer framebuffer;

	// 延迟渲染的四边形
	GMuint quadVAO = 0;
	GMuint quadVBO = 0;
	// 渲染状态
	GMGLDeferredRenderState renderState = GMGLDeferredRenderState::PassingGeometry;

	// 延迟渲染分组
	Vector<GMGameObject*> deferredRenderingGameObjects;
	Vector<GMGameObject*> forwardRenderingGameObjects;

	GMint stencilRenderModeCache = GMStates_RenderOptions::FORWARD;
	GMRenderMode renderMode = GMStates_RenderOptions::FORWARD;
	GMint createStencilRef = 0;
	GMint useStencilRef = 0;

	// 混合绘制
	GMRenderMode renderModeForBlend = GMStates_RenderOptions::FORWARD;
	GMS_BlendFunc blendsfactor;
	GMS_BlendFunc blenddfactor;
	bool isBlending = false;

	// 是否进入绘制模式
#if _DEBUG
	GMint drawingLevel = 0;
#endif
};

class GMGLGraphicEngine : public GMObject, public IGraphicEngine
{
	DECLARE_PRIVATE(GMGLGraphicEngine)

public:
	GMGLGraphicEngine() = default;
	virtual ~GMGLGraphicEngine();

public:
	virtual void start() override;
	virtual void newFrame() override;
	virtual bool event(const GameMachineMessage& e) override;
	virtual void drawObjects(GMGameObject *objects[], GMuint count, GMBufferMode bufferMode) override;
	virtual void update(GMUpdateDataType type) override;
	virtual void addLight(const GMLight& light) override;
	virtual void removeLights();
	virtual void clearStencil() override;
	virtual void beginCreateStencil() override;
	virtual void endCreateStencil() override;
	virtual void beginUseStencil(bool inverse) override;
	virtual void endUseStencil() override;
	virtual void beginBlend(GMS_BlendFunc sfactor, GMS_BlendFunc dfactor) override;
	virtual void endBlend() override;

public:
	void setShaderLoadCallback(IShaderLoadCallback* cb) { D(d); d->shaderLoadCallback = cb; }
	IRender* getRender(GMModelType objectType);
	void setViewport(const GMRect& rect);
	void updateShader();

public:
	inline GMGLDeferredRenderState getRenderState() { D(d); return d->renderState; }
	inline bool isBlending() { D(d); return d->isBlending; }
	GMS_BlendFunc blendsfactor() { D(d); return d->blendsfactor; }
	GMS_BlendFunc blenddfactor() { D(d); return d->blenddfactor; }
	GMRenderMode getCurrentRenderMode() { D(d); return d->renderMode; }

	GMGLShaderProgram* getShaderProgram()
	{
		D(d);
		if (getCurrentRenderMode() == GMStates_RenderOptions::FORWARD)
			return d->forwardShaderProgram;
		GM_ASSERT(getCurrentRenderMode() == GMStates_RenderOptions::DEFERRED);
		if (getRenderState() != GMGLDeferredRenderState::PassingLight)
			return d->deferredShaderProgram[DEFERRED_GEOMETRY_PASS_SHADER];
		return d->deferredShaderProgram[DEFERRED_LIGHT_PASS_SHADER];
	}

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

private:
	void directDraw(GMGameObject *objects[], GMuint count);
	void forwardDraw(GMGameObject *objects[], GMuint count);
	void activateLightsIfNecessary();
	void createDeferredRenderQuad();
	void renderDeferredRenderQuad();
	void disposeDeferredRenderQuad();
	void updateProjection();
	void updateView();
	void installShaders();
	void activateLights(const Vector<GMLight>& lights);
	bool refreshGBuffer();
	bool refreshFramebuffer();
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