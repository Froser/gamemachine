#include "stdafx.h"
#include "shader_constants.h"
#include <algorithm>
#include "gmglgraphic_engine.h"
#include "gmglfunc.h"
#include "gmdatacore/object.h"
#include "gmengine/elements/gameobject.h"
#include "gmengine/controller/graphic_engine.h"
#include "gmgltexture.h"
#include "gmengine/elements/gameworld.h"
#include "gmengine/elements/gamelight.h"
#include "gmglobjectpainter.h"

struct __shadowSourcePred
{
	bool operator() (GameLight* light)
	{
		return light->getShadowSource();
	}
};

GMGLGraphicEngine::GMGLGraphicEngine()
	: m_world(nullptr)
	, m_shadowMapping(*this)
	, m_settings(nullptr)
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
	GameLight* shadowSourceLight = getShadowSourceLight();
	if (shadowSourceLight)
	{
		m_shadowMapping.beginDrawDepthBuffer(shadowSourceLight);
		drawObjectsOnce(drawingList, !!shadowSourceLight);
		m_shadowMapping.endDrawDepthBuffer();
	}

	drawObjectsOnce(drawingList, !!shadowSourceLight);
}

void GMGLGraphicEngine::drawObjectsOnce(DrawingList& drawingList, bool shadowOn)
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

		Object* coreObj = item.gameObject->getObject();
		GMGLObjectPainter* painter = static_cast<GMGLObjectPainter*>(coreObj->getPainter());

		if (!shadowMapping)
		{
			setEyeViewport(shadowOn);
			shadowTexture(shadowOn);

			painter->setWorld(m_world);
		}
		else
		{
			painter->setWorld(nullptr);
		}

		painter->draw();
	}

	if (!shadowMapping)
		endSetSky();
}

void GMGLGraphicEngine::setEyeViewport(bool shadowOn)
{
	static const vmath::mat4 biasMatrix = vmath::mat4(
		vmath::vec4(0.5f, 0.0f, 0.0f, 0.0f),
		vmath::vec4(0.0f, 0.5f, 0.0f, 0.0f),
		vmath::vec4(0.0f, 0.0f, 0.5f, 0.0f),
		vmath::vec4(0.5f, 0.5f, 0.5f, 1.0f));

	if (shadowOn)
	{
		const GMGLShadowMapping::State& state = m_shadowMapping.getState();
		glViewport(state.viewport[0], state.viewport[1], state.viewport[2], state.viewport[3]);
		GMGL::uniformMatrix4(m_shaders, biasMatrix * state.lightProjectionMatrix * state.lightViewMatrix, GMSHADER_SHADOW_MATRIX);
	}
	GMGL::perspective(30, 2, 1, 2000, m_shaders, GMSHADER_PROJECTION_MATRIX);
}

void GMGLGraphicEngine::shadowTexture(bool shadowOn)
{
	if (shadowOn)
	{
		GMGL::uniformTextureIndex(m_shaders, TextureTypeShadow, getTextureUniformName(TextureTypeShadow));
		glActiveTexture(TextureTypeShadow + GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_shadowMapping.getDepthTexture());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		GMGL::disableTexture(m_shaders, getTextureUniformName(TextureTypeShadow));
	}
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

GameLight* GMGLGraphicEngine::getShadowSourceLight()
{
	std::vector<GameLight*>& lights = getWorld()->getLights();
	auto result = std::find_if(lights.begin(), lights.end(), __shadowSourcePred());
	if (result == lights.end())
		return nullptr;

	return *result;
}

GMGLShaders& GMGLGraphicEngine::getShaders()
{
	return m_shaders;
}

GMGLShadowMapping& GMGLGraphicEngine::getShadowMapping()
{
	return m_shadowMapping;
}

void GMGLGraphicEngine::updateCameraView(const CameraLookAt& lookAt)
{
	m_shaders.useProgram();
	GMGL::lookAt(lookAt, m_shaders, GMSHADER_VIEW_MATRIX);
	GMGL::cameraPosition(lookAt, m_shaders, GMSHADER_VIEW_POSITION);
}

GameWorld* GMGLGraphicEngine::getWorld()
{
	return m_world;
}

ResourceContainer* GMGLGraphicEngine::getResourceContainer()
{
	return &m_resourceContainer;
}

GraphicSettings* GMGLGraphicEngine::getGraphicSettings()
{
	ASSERT(m_settings);
	return m_settings;
}

void GMGLGraphicEngine::setGraphicSettings(GraphicSettings* settings)
{
	m_settings = settings;
}