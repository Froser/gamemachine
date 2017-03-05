﻿#ifndef __GMGLGRAPHIC_ENGINE_H__
#define __GMGLGRAPHIC_ENGINE_H__
#include "common.h"
#include "gmengine/controllers/graphic_engine.h"
#include "gmglshaders.h"
#include "gmgllight.h"
#include "gmglshadowmapping.h"
#include "gmengine/controllers/resource_container.h"
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
	GMGLShadowMapping* getShadowMapping();
	GameWorld* getWorld();

	void registerShader(ChildObject::ObjectType objectType, AUTORELEASE GMGLShaders* shaders);
	GMGLShaders* getShaders(ChildObject::ObjectType objectType);

private:
	void applyGraphicSettings();
	void updateMatrices(const CameraLookAt& lookAt);
	void drawObjectsOnce(DrawingList& drawingList, bool shadowOn);
	void setEyeViewport(bool shadowOn, GMGLShaders& shaders);
	void shadowTexture(bool shadowOn, GMGLShaders& shaders);
	GameLight* getShadowSourceLight();

private:
	std::map<ChildObject::ObjectType, GMGLShaders*> m_allShaders;
	AutoPtr<GMGLShadowMapping> m_shadowMapping;
	GameWorld* m_world;
	ResourceContainer m_resourceContainer;
	GraphicSettings* m_settings;

	vmath::mat4 m_viewMatrix;
	vmath::mat4 m_projectionMatrix;
};

END_NS
#endif