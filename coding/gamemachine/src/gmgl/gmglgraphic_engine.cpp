#include "stdafx.h"
#include "shader_constants.h"
#include "gmglgraphic_engine.h"
#include "gmglfunc.h"
#include "gmdatacore/object.h"
#include "gmengine/elements/gameobject.h"
#include "gmengine/controller/graphic_engine.h"
#include "gmgltexture.h"
#include "gmengine/elements/gameworld.h"

GMGLGraphicEngine::GMGLGraphicEngine()
	: m_lightController(m_shaders)
	, m_world(nullptr)
	, m_shadowMapping(*this)
{
}

void GMGLGraphicEngine::initialize(GameWorld* world)
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	m_world = world;
	m_shadowMapping.init();
}

void GMGLGraphicEngine::newFrame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GMGLGraphicEngine::drawObjects(DrawingList& drawingList)
{
	m_shadowMapping.beginDrawDepthBuffer();
	drawObjectsOnce(drawingList);
	m_shadowMapping.endDrawDepthBuffer();

	drawObjectsOnce(drawingList);
}

void GMGLGraphicEngine::drawObjectsOnce(DrawingList& drawingList)
{
	bool shadowMapping = m_shadowMapping.hasBegun();
	GMGLShaders& shaders = shadowMapping ? m_shadowMapping.getShaders() : m_shaders;
	shaders.useProgram();

	if (!shadowMapping)
		beginSetSky();

	for (auto iter = drawingList.begin(); iter != drawingList.end(); iter++)
	{
		DrawingItem& item = *iter;
		GMGL::uniformMatrix4(shaders, item.trans, GMSHADER_MODEL_MATRIX);

		if (!shadowMapping)
		{
			setEyeViewport();
			activeShadowTexture();
		}

		Object* coreObj = item.gameObject->getObject();
		ObjectPainter* painter = coreObj->getPainter();
		painter->draw();
	}

	if (!shadowMapping)
		endSetSky();
}

void GMGLGraphicEngine::setEyeViewport()
{
	static const vmath::mat4 biasMatrix = vmath::mat4(
		vmath::vec4(0.5f, 0.0f, 0.0f, 0.0f),
		vmath::vec4(0.0f, 0.5f, 0.0f, 0.0f),
		vmath::vec4(0.0f, 0.0f, 0.5f, 0.0f),
		vmath::vec4(0.5f, 0.5f, 0.5f, 1.0f));

	const GMGLShadowMapping::State& state = m_shadowMapping.getState();
	glViewport(state.viewport[0], state.viewport[1], state.viewport[2], state.viewport[3]);
	GMGL::uniformMatrix4(m_shaders, biasMatrix * state.lightProjectionMatrix * state.lightViewMatrix, GMSHADER_SHADOW_MATRIX);
	GMGL::perspective(30, 2, 1, 2000, m_shaders, GMSHADER_PROJECTION_MATRIX);
}

void GMGLGraphicEngine::activeShadowTexture()
{
	GMGL::uniformTextureIndex(m_shaders, TextureTypeShadow, getTextureUniformName(TextureTypeShadow));
	glActiveTexture(TextureTypeShadow + GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_shadowMapping.getDepthTexture());
	glGenerateMipmap(GL_TEXTURE_2D);
}

void GMGLGraphicEngine::beginSetSky()
{
	GameObject* sky = getWorld()->getSky();
	if (sky)
	{
		GMGL::uniformTextureIndex(m_shaders, TextureTypeReflectionCubeMap, getTextureUniformName(TextureTypeReflectionCubeMap));
		TextureInfo& info = sky->getObject()->getComponents()[0]->getMaterial().textures[0];
		ASSERT(info.texture);
		info.texture->beginTexture(TextureTypeReflectionCubeMap);
	}
}

void GMGLGraphicEngine::endSetSky()
{
	GameObject* sky = getWorld()->getSky();
	if (sky)
	{
		TextureInfo& info = sky->getObject()->getComponents()[0]->getMaterial().textures[0];
		info.texture->endTexture();
	}
}

GMGLShaders& GMGLGraphicEngine::getShaders()
{
	return m_shaders;
}

GMGLShadowMapping& GMGLGraphicEngine::getShadowMapping()
{
	return m_shadowMapping;
}

void GMGLGraphicEngine::updateCameraView(Camera& camera)
{
	GMGL::lookAt(camera, m_shaders, GMSHADER_VIEW_MATRIX);
}

GameWorld* GMGLGraphicEngine::getWorld()
{
	return m_world;
}

ILightController& GMGLGraphicEngine::getLightController()
{
	return m_lightController;
}