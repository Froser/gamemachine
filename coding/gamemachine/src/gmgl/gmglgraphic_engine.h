#ifndef __GMGLGRAPHIC_ENGINE_H__
#define __GMGLGRAPHIC_ENGINE_H__
#include "common.h"
#include "gmglshaders.h"
#include "gmengine/controllers/resource_container.h"
#include "foundation/utilities/utilities.h"
#include <map>
BEGIN_NS

class Camera;
class GMGameWorld;
class GameLight;
struct IRender;

GM_PRIVATE_OBJECT(GMGLGraphicEngine)
{
	std::map<Mesh::MeshesType, GMGLShaders*> allShaders;
	std::map<Mesh::MeshesType, IRender*> allRenders;
	GMGameWorld* world;
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
	virtual void setCurrentWorld(GMGameWorld*) override;
	virtual void newFrame() override;
	virtual void drawObject(GMGameObject* obj) override;
	virtual void updateCameraView(const CameraLookAt& lookAt) override;
	virtual ResourceContainer* getResourceContainer() override;

public:
	GMGameWorld* getWorld();

	void registerShader(Mesh::MeshesType objectType, AUTORELEASE GMGLShaders* shaders);
	GMGLShaders* getShaders(Mesh::MeshesType objectType);

	void registerRender(Mesh::MeshesType objectType, AUTORELEASE IRender* render);
	IRender* getRender(Mesh::MeshesType objectType);

private:
	void applyGraphicSettings();
	void updateMatrices(const CameraLookAt& lookAt);
	void drawObjectOnce(GMGameObject* object);
};

END_NS
#endif