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
	virtual bool onLoadShader(const GMMeshType type, GMGLShaderProgram* shaderProgram) = 0;
};

GM_PRIVATE_OBJECT(GMGLGraphicEngine)
{
	bool needRefreshLights = true;
	GMGLRenderMode renderMode = GMGLRenderMode::ForwardRendering;
	Vector<GMLight> lights;
	Map<GMMeshType, GMGLShaderProgram*> allShaders;
	Map<GMMeshType, IRender*> allRenders;
	IShaderLoadCallback* shaderLoadCallback = nullptr;
	ResourceContainer resourceContainer;
	GraphicSettings* settings = nullptr;
	linear_math::Matrix4x4 viewMatrix;
	linear_math::Matrix4x4 projectionMatrix;
	GMGLGBuffer gbuffer;
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
	void setViewport(const GMRect& rect);
	void registerShader(GMMeshType objectType, AUTORELEASE GMGLShaderProgram* shaders);
	void updateMatrices(const CameraLookAt& lookAt);
	void installShaders();
	bool loadDefaultShaders(const GMMeshType type, GMGLShaderProgram* shaderProgram);
	void activateLight(const Vector<GMLight>& lights);
};

END_NS
#endif