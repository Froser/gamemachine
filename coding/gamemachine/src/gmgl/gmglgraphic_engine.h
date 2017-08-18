#ifndef __GMGLGRAPHIC_ENGINE_H__
#define __GMGLGRAPHIC_ENGINE_H__
#include "common.h"
#include "gmglshaderprogram.h"
#include "gmengine/resource_container.h"
#include "foundation/utilities/utilities.h"
#include <map>
#include "gmglgbuffer.h"
BEGIN_NS

class Camera;
class GMGameWorld;
class GameLight;
struct IRender;

enum class GMGLRenderMode
{
	ForwardRendering,
	DeferredRendering,
};

GM_INTERFACE(IShaderLoadCallback)
{
	virtual bool onLoadForwardShader(const GMMeshType type, GMGLShaderProgram& shaderProgram) = 0;
	virtual bool onLoadDeferredGeometryPassShader(const GMMeshType type, GMGLShaderProgram& geometryPassProgram) = 0;
	virtual bool onLoadDeferredLightPassShader(GMGLShaderProgram& lightPassProgram) = 0;
};

GM_PRIVATE_OBJECT(GMGLGraphicEngine)
{
	bool needRefreshLights = true;
	GMGLRenderMode renderMode = GMGLRenderMode::ForwardRendering;
	Vector<GMLight> lights;
	Map<GMMeshType, GMGLShaderProgram* > forwardRenderingShaders;
	Map<GMMeshType, GMGLShaderProgram* > deferredGeometryPassShader;
	GMGLShaderProgram* deferredLightPassShader;
	Map<GMMeshType, IRender*> allRenders;
	IShaderLoadCallback* shaderLoadCallback = nullptr;
	ResourceContainer resourceContainer;
	GraphicSettings* settings = nullptr;
	linear_math::Matrix4x4 viewMatrix;
	linear_math::Matrix4x4 projectionMatrix;
	GMGLGBuffer gbuffer;
	// 延迟渲染的四边形
	GMuint quadVAO = 0;
	GMuint quadVBO = 0;
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
	virtual void event(const GameMachineMessage& e) override;
	virtual void drawObjects(GMGameObject *objects[], GMuint count) override;
	virtual void updateCameraView(const CameraLookAt& lookAt) override;
	virtual ResourceContainer* getResourceContainer() override;
	virtual void addLight(const GMLight& light) override;
	virtual void beginCreateStencil() override;
	virtual void endCreateStencil() override;
	virtual void beginUseStencil(bool inverse) override;
	virtual void endUseStencil() override;

public:
	GMGLShaderProgram* getShaders(GMMeshType objectType);
	void setShaderLoadCallback(IShaderLoadCallback* cb) { D(d); d->shaderLoadCallback = cb; }
	void registerRender(GMMeshType objectType, AUTORELEASE IRender* render);
	IRender* getRender(GMMeshType objectType);
	void setRenderMode(GMGLRenderMode mode);

private:
	void initDeferredRenderQuad();
	void disposeDeferredRenderQuad();
	void setViewport(const GMRect& rect);
	void registerForwardRenderingShader(GMMeshType objectType, AUTORELEASE GMGLShaderProgram* forwardShaderProgram);
	void registerGeometryPassShader(GMMeshType objectType, AUTORELEASE GMGLShaderProgram* deferredGeometryPassProgram);
	void registerLightPassShader(AUTORELEASE GMGLShaderProgram* deferredLightPassProgram);
	void updateMatrices(const CameraLookAt& lookAt);
	void installShaders();
	bool loadDefaultForwardShader(const GMMeshType type, GMGLShaderProgram* shaderProgram);
	bool loadDefaultDeferredGeometryPassShader(const GMMeshType type, GMGLShaderProgram* shaderProgram);
	bool loadDefaultDeferredLightPassShader(GMGLShaderProgram* shaderProgram);
	void activateLight(const Vector<GMLight>& lights);
	bool refreshGBuffer();
	void forwardRender(GMGameObject* objects[], GMuint count);
	void geometryPass(GMGameObject *objects[], GMuint count);
	void lightPass(GMGameObject *objects[], GMuint count);

public:
	static void newFrameOnCurrentContext();
};

END_NS
#endif