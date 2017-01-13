#ifndef __GMGLGRAPHIC_ENGINE_H__
#define __GMGLGRAPHIC_ENGINE_H__
#include "common.h"
#include "gmengine/controller/graphic_engine.h"
#include "gmglshaders.h"
#include "gmgllight.h"
#include "gmglshadowmapping.h"
#include "gmengine/controller/resource_container.h"
#include <map>
#include "utilities/camera.h"
BEGIN_NS

class Camera;
class GameWorld;
class GameLight;
class GMGLGraphicEngine : public IGraphicEngine
{
public:
	GMGLGraphicEngine();
	virtual ~GMGLGraphicEngine();

public:
	virtual void initialize(GameWorld*) override;
	virtual void newFrame() override;
	virtual void drawObjects(DrawingList& drawingList) override;
	virtual void updateCameraView(const CameraLookAt& lookAt) override;
	virtual ResourceContainer* getResourceContainer() override;
	virtual GraphicSettings* getGraphicSettings() override;
	virtual void setGraphicSettings(GraphicSettings* settings) override;

public:
	GMGLShadowMapping& getShadowMapping();
	GameWorld* getWorld();

	void registerShader(Object::ObjectType objectType, AUTORELEASE GMGLShaders* shaders);
	GMGLShaders* getShaders(Object::ObjectType objectType);

private:
	void drawObjectsOnce(DrawingList& drawingList, bool shadowOn);
	void setEyeViewport(bool shadowOn, GMGLShaders& shaders);
	void shadowTexture(bool shadowOn, GMGLShaders& shaders);
	void beginSetSky(GMGLShaders& shaders);
	void endSetSky();
	GameLight* getShadowSourceLight();

private:
	std::map<Object::ObjectType, GMGLShaders*> m_allShaders;
	GMGLShadowMapping m_shadowMapping;
	GameWorld* m_world;
	ResourceContainer m_resourceContainer;
	GraphicSettings* m_settings;
};

END_NS
#endif