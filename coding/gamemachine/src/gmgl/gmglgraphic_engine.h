#ifndef __GMGLGRAPHIC_ENGINE_H__
#define __GMGLGRAPHIC_ENGINE_H__
#include "common.h"
#include "gmengine/controller/graphic_engine.h"
#include "gmglshaders.h"
#include "gmgllight.h"
#include "gmglshadowmapping.h"
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

public:
	GMGLShaders& getShaders();
	GMGLShadowMapping& getShadowMapping();
	GameWorld* getWorld();

private:
	void drawObjectsOnce(DrawingList& drawingList);
	void setEyeViewport();
	void bindAllTextures();
	void activeShadowTexture();

private:
	GMGLShaders m_shaders;
	GMGLLight m_lightController;
	GMGLShadowMapping m_shadowMapping;
	GameWorld* m_world;
};

END_NS
#endif