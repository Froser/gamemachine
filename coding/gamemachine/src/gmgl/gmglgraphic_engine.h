#ifndef __GMGLGRAPHIC_ENGINE_H__
#define __GMGLGRAPHIC_ENGINE_H__
#include "common.h"
#include "gmengine/controller/graphic_engine.h"
#include "gmglshaders.h"
#include "gmgllight.h"
#include "gmglshadowmapping.h"
#include "gmengine/controller/resource_container.h"
BEGIN_NS

class Camera;
class GameWorld;
class GameLight;
class GMGLGraphicEngine : public IGraphicEngine
{
public:
	GMGLGraphicEngine();

public:
	virtual void initialize(GameWorld*) override;
	virtual void newFrame() override;
	virtual void drawObjects(DrawingList& drawingList) override;
	virtual void updateCameraView(const CameraLookAt& lookAt) override;
	virtual ResourceContainer* getResourceContainer() override;
	virtual GraphicSettings& getGraphicSettings() override;

public:
	GMGLShaders& getShaders();
	GMGLShadowMapping& getShadowMapping();
	GameWorld* getWorld();

private:
	void drawObjectsOnce(DrawingList& drawingList, bool shadowOn);
	void setEyeViewport(bool shadowOn);
	void shadowTexture(bool shadowOn);
	void beginSetSky();
	void endSetSky();
	GameLight* getShadowSourceLight();

private:
	GMGLShaders m_shaders;
	GMGLShadowMapping m_shadowMapping;
	GameWorld* m_world;
	ResourceContainer m_resourceContainer;
	GraphicSettings m_settings;
};

END_NS
#endif