#include "stdafx.h"
#include "shader_constants.h"
#include <algorithm>
#include "gmglgraphic_engine.h"
#include "gmglfunc.h"
#include "gmdatacore/object.h"
#include "gmengine/elements/gameobject.h"
#include "gmengine/controllers/graphic_engine.h"
#include "gmgltexture.h"
#include "gmengine/elements/gameworld.h"
#include "gmglobjectpainter.h"
#include "gmengine/elements/character.h"
#include "renders/gmgl_render.h"

GMGLGraphicEngine::GMGLGraphicEngine()
{
	D(d);
	d->world = nullptr;
	d->settings = nullptr;
}

GMGLGraphicEngine::~GMGLGraphicEngine()
{
	D(d);
	for (auto iter = d->allShaders.begin(); iter != d->allShaders.end(); iter++)
	{
		if ((*iter).second)
			delete (*iter).second;
	}

	for (auto iter = d->allRenders.begin(); iter != d->allRenders.end(); iter++)
	{
		if ((*iter).second)
			delete (*iter).second;
	}
}

void GMGLGraphicEngine::setCurrentWorld(GameWorld* world)
{
	D(d);
	d->world = world;
}

void GMGLGraphicEngine::newFrame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GMGLGraphicEngine::drawObject(GameObject* object)
{
	applyGraphicSettings();
	drawObjectOnce(object);
}

void GMGLGraphicEngine::applyGraphicSettings()
{
	glLineWidth(0);
	glDepthFunc(GL_LEQUAL);
	glFrontFace(GL_CW);
}

void GMGLGraphicEngine::drawObjectOnce(GameObject* object)
{
	D(d);
	static GMfloat trans[] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	GMGLShaders* lastShaders = nullptr;
	object->onBeforeDraw();
	Object* coreObj = object->getObject();
	coreObj->getPainter()->draw(trans);
}

void GMGLGraphicEngine::updateCameraView(const CameraLookAt& lookAt)
{
	D(d);
	updateMatrices(lookAt);

	BEGIN_ENUM(i, Mesh::ObjectTypeBegin, Mesh::ObjectTypeEnd)
	{
		IRender* render = getRender(i);
		Mesh dummy;
		dummy.setType(i);

		render->begin(this, &dummy, nullptr);
		render->updateVPMatrices(d->projectionMatrix, d->viewMatrix, lookAt);
		render->end();
	}
	END_ENUM
}

void GMGLGraphicEngine::updateMatrices(const CameraLookAt& lookAt)
{
	D(d);
	Character* character = getWorld()->getMajorCharacter();

	//TODO 
	d->projectionMatrix = character->getFrustum().getPerspective();
	d->viewMatrix = getViewMatrix(lookAt);

	character->getFrustum().updateViewMatrix(d->viewMatrix, d->projectionMatrix);
	character->getFrustum().update();
}

GameWorld* GMGLGraphicEngine::getWorld()
{
	D(d);
	return d->world;
}

void GMGLGraphicEngine::registerShader(Mesh::MeshesType objectType, AUTORELEASE GMGLShaders* shaders)
{
	D(d);
	d->allShaders[objectType] = shaders;
}

GMGLShaders* GMGLGraphicEngine::getShaders(Mesh::MeshesType objectType)
{
	D(d);
	if (d->allShaders.find(objectType) == d->allShaders.end())
		return nullptr;
	
	return d->allShaders[objectType];
}

void GMGLGraphicEngine::registerRender(Mesh::MeshesType objectType, AUTORELEASE IRender* render)
{
	D(d);
	d->allRenders[objectType] = render;
}

IRender* GMGLGraphicEngine::getRender(Mesh::MeshesType objectType)
{
	D(d);
	if (d->allRenders.find(objectType) == d->allRenders.end())
		return nullptr;

	return d->allRenders[objectType];
}

ResourceContainer* GMGLGraphicEngine::getResourceContainer()
{
	D(d);
	return &d->resourceContainer;
}

GraphicSettings* GMGLGraphicEngine::getGraphicSettings()
{
	D(d);
	ASSERT(d->settings);
	return d->settings;
}

void GMGLGraphicEngine::setGraphicSettings(GraphicSettings* settings)
{
	D(d);
	d->settings = settings;
}