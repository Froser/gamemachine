#ifndef __GMGLGRAPHIC_ENGINE_H__
#define __GMGLGRAPHIC_ENGINE_H__
#include <gmcommon.h>
#include <gmassets.h>
#include "gmglshaderprogram.h"
#include <gamemachine.h>
#include <map>
#include "gmglgbuffer.h"
#include "renders/gmgl_renders_lightpass.h"
BEGIN_NS

class Camera;
class GMGameWorld;
class GameLight;
struct IRender;

extern "C"
{
	extern GLenum s_glErrCode;
}

#define GM_CHECK_GL_ERROR() GM_ASSERT((s_glErrCode = glGetError()) == GL_NO_ERROR);
#define GM_BEGIN_CHECK_GL_ERROR { GM_CHECK_GL_ERROR()
#define GM_END_CHECK_GL_ERROR GM_CHECK_GL_ERROR() }

GM_INTERFACE(IShaderLoadCallback)
{
	virtual void onLoadForwardShader(const GMMeshType type, GMGLShaderProgram& shaderProgram) = 0;
	virtual void onLoadDeferredPassShader(GMGLDeferredRenderState state, GMGLShaderProgram& shaderProgram) = 0;
	virtual void onLoadDeferredLightPassShader(GMGLShaderProgram& lightPassProgram) = 0;
	virtual void onLoadEffectsShader(GMGLShaderProgram& effectsProgram) = 0;
};

GM_PRIVATE_OBJECT(GMGLGraphicEngine)
{
	bool needRefreshLights = true;
	Vector<GMLight> lights;

	// 渲染器
	Map<GMMeshType, IRender*> allRenders;
	GMGLRenders_LightPass* lightPassRender = nullptr;

	// 著色器程序
	Map<GMMeshType, GMGLShaderProgram* > forwardRenderingShaders;
	Map<GMGLDeferredRenderState, GMGLShaderProgram* > deferredCommonPassShaders;
	GMGLShaderProgram* deferredLightPassShader = nullptr;
	GMGLShaderProgram* effectsShader = nullptr;

	IShaderLoadCallback* shaderLoadCallback = nullptr;
	GraphicSettings* settings = nullptr;
	linear_math::Matrix4x4 viewMatrix;
	linear_math::Matrix4x4 projectionMatrix;

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
	bool isBlending = false;
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
	virtual void drawObjects(GMGameObject *objects[], GMuint count) override;
	virtual void updateCameraView(const CameraLookAt& lookAt) override;
	virtual void addLight(const GMLight& light) override;
	virtual void removeLights();
	virtual void clearStencil() override;
	virtual void beginCreateStencil() override;
	virtual void endCreateStencil() override;
	virtual void beginUseStencil(bool inverse) override;
	virtual void endUseStencil() override;
	virtual void beginBlend() override;
	virtual void endBlend() override;
	virtual void beginFullRendering() override;
	virtual void endFullRendering() override;

public:
	GMGLShaderProgram* getShaders(GMMeshType objectType);
	void setShaderLoadCallback(IShaderLoadCallback* cb) { D(d); d->shaderLoadCallback = cb; }
	void registerRender(GMMeshType objectType, AUTORELEASE IRender* render);
	IRender* getRender(GMMeshType objectType);
	void setViewport(const GMRect& rect);

public:
	inline GMGLShaderProgram* getLightPassShader() { D(d); return d->deferredLightPassShader; }
	inline void setRenderState(GMGLDeferredRenderState state) { D(d); d->renderState = state; }
	inline GMGLDeferredRenderState getRenderState() { D(d); return d->renderState; }
	inline bool isBlending() { D(d); return d->isBlending; }

private:
	void refreshForwardRenderLights();
	void refreshDeferredRenderLights();
	void createDeferredRenderQuad();
	void renderDeferredRenderQuad();
	void disposeDeferredRenderQuad();
	void registerForwardRenderingShader(GMMeshType objectType, AUTORELEASE GMGLShaderProgram* forwardShaderProgram);
	void registerLightPassShader(AUTORELEASE GMGLShaderProgram* deferredLightPassProgram);
	void registerEffectsShader(AUTORELEASE GMGLShaderProgram* effectsShader);
	void registerCommonPassShader(GMGLDeferredRenderState state, AUTORELEASE GMGLShaderProgram* shaderProgram);
	void updateMatrices(const CameraLookAt& lookAt);
	void installShaders();
	void activateForwardRenderLight(const Vector<GMLight>& lights);
	void activateLightPassLight(const Vector<GMLight>& lights);
	bool refreshGBuffer();
	bool refreshFramebuffer();
	void forwardRender(GMGameObject* objects[], GMuint count);
	void geometryPass(Vector<GMGameObject*>& objects);
	void lightPass();
	void updateVPMatrices(const CameraLookAt& lookAt);
	void groupGameObjects(GMGameObject *objects[], GMuint count);
	void viewGBufferFrameBuffer();

public:
	static void newFrameOnCurrentFramebuffer();
	static void clearStencilOnCurrentFramebuffer();
};

END_NS
#endif