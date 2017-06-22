#ifndef __GMGLGRAPHIC_ENGINE_H__
#define __GMGLGRAPHIC_ENGINE_H__
#include "common.h"
#include "gmglshaderprogram.h"
#include "gmengine/resource_container.h"
#include "foundation/utilities/utilities.h"
#include <map>
BEGIN_NS

class Camera;
class GMGameWorld;
class GameLight;
struct IRender;

struct IShaderLoadCallback : public IGMInterface
{
	virtual bool onLoadShader(const GMMeshType type, GMGLShaderProgram* shaderProgram) = 0;
};

GM_PRIVATE_OBJECT(GMGLGraphicEngine)
{
	std::map<GMMeshType, GMGLShaderProgram*> allShaders;
	std::map<GMMeshType, IRender*> allRenders;
	GMGameWorld* world = nullptr;
	IShaderLoadCallback* shaderLoadCallback = nullptr;
	ResourceContainer resourceContainer;
	GraphicSettings* settings;
	linear_math::Matrix4x4 viewMatrix;
	linear_math::Matrix4x4 projectionMatrix;
};

class GMGLGraphicEngine : public GMObject, public IGraphicEngine
{
	DECLARE_PRIVATE(GMGLGraphicEngine)

public:
	GMGLGraphicEngine();
	virtual ~GMGLGraphicEngine();

public:
	virtual void start() override;
	virtual void setCurrentWorld(GMGameWorld*) override;
	virtual void newFrame() override;
	virtual void setViewport(const GMRect& rect) override;
	virtual void drawObject(GMGameObject* obj) override;
	virtual void updateCameraView(const CameraLookAt& lookAt) override;
	virtual ResourceContainer* getResourceContainer() override;

public:
	GMGameWorld* getWorld();

	GMGLShaderProgram* getShaders(GMMeshType objectType);
	void setShaderLoadCallback(IShaderLoadCallback* cb) { D(d); d->shaderLoadCallback = cb; }

	void registerRender(GMMeshType objectType, AUTORELEASE IRender* render);
	IRender* getRender(GMMeshType objectType);

private:
	void registerShader(GMMeshType objectType, AUTORELEASE GMGLShaderProgram* shaders);
	void applyGraphicSettings();
	void updateMatrices(const CameraLookAt& lookAt);
	void drawObjectOnce(GMGameObject* object);
	void installShaders();
	bool loadDefaultShaders(const GMMeshType type, GMGLShaderProgram* shaderProgram);
};

END_NS
#endif