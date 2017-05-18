#ifndef __GMGLGRAPHIC_ENGINE_H__
#define __GMGLGRAPHIC_ENGINE_H__
#include "common.h"
#include "gmengine/controllers/graphic_engine.h"
#include "gmglshaders.h"
#include "gmengine/controllers/resource_container.h"
#include "utilities/utilities.h"
#include <map>
BEGIN_NS

class Camera;
class GameWorld;
class GameLight;
struct IRender;

struct GMGLGraphicEnginePrivate
{
	std::map<ChildObject::ObjectType, GMGLShaders*> allShaders;
	std::map<ChildObject::ObjectType, IRender*> allRenders;
	GameWorld* world;
	ResourceContainer resourceContainer;
	GraphicSettings* settings;
	vmath::mat4 viewMatrix;
	vmath::mat4 projectionMatrix;
};

class GMGLGraphicEngine : public IGraphicEngine
{
	DEFINE_PRIVATE(GMGLGraphicEngine)

public:
	GMGLGraphicEngine();
	virtual ~GMGLGraphicEngine();

public:
	virtual void setCurrentWorld(GameWorld*) override;
	virtual void newFrame() override;
	virtual void drawObjects(DrawingList& drawingList) override;
	virtual void updateCameraView(const CameraLookAt& lookAt) override;
	virtual ResourceContainer* getResourceContainer() override;
	virtual GraphicSettings* getGraphicSettings() override;
	virtual void setGraphicSettings(GraphicSettings* settings) override;

public:
	GameWorld* getWorld();

	void registerShader(ChildObject::ObjectType objectType, AUTORELEASE GMGLShaders* shaders);
	GMGLShaders* getShaders(ChildObject::ObjectType objectType);

	void registerRender(ChildObject::ObjectType objectType, AUTORELEASE IRender* render);
	IRender* getRender(ChildObject::ObjectType objectType);

private:
	void applyGraphicSettings();
	void updateMatrices(const CameraLookAt& lookAt);
	void drawObjectsOnce(DrawingList& drawingList);
};

END_NS
#endif