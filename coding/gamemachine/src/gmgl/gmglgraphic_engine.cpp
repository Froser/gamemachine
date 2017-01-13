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

GMGLGraphicEngine::~GMGLGraphicEngine()
{
	for (auto iter = m_allShaders.begin(); iter != m_allShaders.end(); iter++)
	{
		if ((*iter).second)
			delete (*iter).second;
	}
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

	for (auto iter = drawingList.begin(); iter != drawingList.end(); iter++)
	{
		Object::ObjectType type = (*iter).gameObject->getObject()->getType();
		GMGLShaders& shaders = shadowMapping ? m_shadowMapping.getShaders() : *getShaders(type);

		shaders.useProgram();
		if (!shadowMapping)
			beginSetSky(shaders);

		DrawingItem& item = *iter;
		GMGL::uniformMatrix4(shaders, item.trans, GMSHADER_MODEL_MATRIX);

		Object* coreObj = item.gameObject->getObject();
		GMGLObjectPainter* painter = static_cast<GMGLObjectPainter*>(coreObj->getPainter());

		if (!shadowMapping)
		{
			setEyeViewport(shadowOn, shaders);
			shadowTexture(shadowOn, shaders);

			painter->setWorld(m_world);
		}
		else
		{
			painter->setWorld(nullptr);
		}

		painter->draw();

		if (!shadowMapping)
			endSetSky();
	}
}

void GMGLGraphicEngine::setEyeViewport(bool shadowOn, GMGLShaders& shaders)
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
		GMGL::uniformMatrix4(shaders, biasMatrix * state.lightProjectionMatrix * state.lightViewMatrix, GMSHADER_SHADOW_MATRIX);
	}
	GMGL::perspective(30, 2, 1, 2000, shaders, GMSHADER_PROJECTION_MATRIX);
}

void GMGLGraphicEngine::shadowTexture(bool shadowOn, GMGLShaders& shaders)
{
	if (shadowOn)
	{
		GMGL::uniformTextureIndex(shaders, TextureTypeShadow, getTextureUniformName(TextureTypeShadow));
		glActiveTexture(TextureTypeShadow + GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_shadowMapping.getDepthTexture());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		GMGL::disableTexture(shaders, getTextureUniformName(TextureTypeShadow));
	}
}

void GMGLGraphicEngine::beginSetSky(GMGLShaders& shaders)
{
	GameObject* sky = getWorld()->getSky();
	if (sky)
	{
		GMGL::uniformTextureIndex(shaders, TextureTypeReflectionCubeMap, getTextureUniformName(TextureTypeReflectionCubeMap));
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

GMGLShadowMapping& GMGLGraphicEngine::getShadowMapping()
{
	return m_shadowMapping;
}

void GMGLGraphicEngine::updateCameraView(const CameraLookAt& lookAt)
{
	BEGIN_ENUM(i, Object::ObjectTypeBegin, Object::ObjectTypeEnd)
	{
		GMGLShaders* shaders = getShaders(i);
		if (!shaders)
			continue;
		shaders->useProgram();
		GMGL::lookAt(lookAt, *shaders, GMSHADER_VIEW_MATRIX);
		GMGL::cameraPosition(lookAt, *shaders, GMSHADER_VIEW_POSITION);
	}
	END_ENUM
}

GameWorld* GMGLGraphicEngine::getWorld()
{
	return m_world;
}

void GMGLGraphicEngine::registerShader(Object::ObjectType objectType, AUTORELEASE GMGLShaders* shaders)
{
	m_allShaders[objectType] = shaders;
}

GMGLShaders* GMGLGraphicEngine::getShaders(Object::ObjectType objectType)
{
	if (m_allShaders.find(objectType) == m_allShaders.end())
		return nullptr;
	
	return m_allShaders[objectType];
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