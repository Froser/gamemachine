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
	virtual bool onLoadShader(const Mesh::MeshesType type, GMGLShaderProgram* shaderProgram) = 0;
};

GM_PRIVATE_OBJECT(GMGLGraphicEngine)
{
	std::map<Mesh::MeshesType, GMGLShaderProgram*> allShaders;
	std::map<Mesh::MeshesType, IRender*> allRenders;
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

	GMGLShaderProgram* getShaders(Mesh::MeshesType objectType);
	void setShaderLoadCallback(IShaderLoadCallback* cb) { D(d); d->shaderLoadCallback = cb; }

	void registerRender(Mesh::MeshesType objectType, AUTORELEASE IRender* render);
	IRender* getRender(Mesh::MeshesType objectType);

private:
	void registerShader(Mesh::MeshesType objectType, AUTORELEASE GMGLShaderProgram* shaders);
	void applyGraphicSettings();
	void updateMatrices(const CameraLookAt& lookAt);
	void drawObjectOnce(GMGameObject* object);
	void installShaders();
	bool loadDefaultShaders(const Mesh::MeshesType type, GMGLShaderProgram* shaderProgram);
};

END_NS
#endif