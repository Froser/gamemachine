#ifndef __GMGLGRAPHIC_ENGINE_H__
#define __GMGLGRAPHIC_ENGINE_H__
#include "common.h"
#include "gmengine/controller/graphic_engine.h"
#include "gmgl/gmglshaders.h"
BEGIN_NS

class Camera;
class GMGLGraphicEngine : public IGraphicEngine
{
public:
	virtual void drawObject(GMfloat transformMatrix[16], GameObject* obj) override;
	virtual void updateCameraView(Camera& camera) override;
	GMGLShaders& getShaders();

private:
	GMGLShaders m_shaders;
};

END_NS
#endif