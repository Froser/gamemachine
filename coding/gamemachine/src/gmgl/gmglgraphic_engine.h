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
class GMGLGraphicEngine : public IGraphicEngine
{
public:
	GMGLGraphicEngine();

public:
	virtual void initialize(GameWorld*) override;
	virtual void newFrame() override;
	virtual void drawObjects(DrawingList& drawingList) override;
	virtual void updateCameraView(Camera& camera) override;
	virtual ILightController& getLightController() override;
	virtual ResourceContainer* getResourceContainer() override;

public:
	GMGLShaders& getShaders();
	GMGLShadowMapping& getShadowMapping();
	GameWorld* getWorld();

private:
	void drawObjectsOnce(DrawingList& drawingList);
	void setEyeViewport();
	void activeShadowTexture();
	void beginSetSky();
	void endSetSky();
	void activateLights();

private:
	GMGLShaders m_shaders;
	GMGLLight m_lightController;
	GMGLShadowMapping m_shadowMapping;
	GameWorld* m_world;
	ResourceContainer m_resourceContainer;
};

END_NS
#endif