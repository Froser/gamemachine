#include "stdafx.h"
#include "shader_constants.h"
#include "gmglgraphic_engine.h"
#include "gmglfunc.h"
#include "gmdatacore/object.h"
#include "gmengine/elements/gameobject.h"

void GMGLGraphicEngine::drawObject(GMfloat transformMatrix[16], GameObject* obj)
{
	Object* coreObj = obj->getObject();
	ObjectPainter* painter = coreObj->getPainter();
	GMGL::transform(m_shaders, transformMatrix, GMSHADER_MODEL_MATRIX);
	painter->draw();
}

GMGLShaders& GMGLGraphicEngine::getShaders()
{
	return m_shaders;
}

void GMGLGraphicEngine::updateCameraView(Camera& camera)
{
	GMGL::lookAt(camera, m_shaders, GMSHADER_VIEW_MATRIX);
}