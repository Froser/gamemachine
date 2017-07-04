#include "stdafx.h"
#include "shader_constants.h"
#include <algorithm>
#include "gmglgraphic_engine.h"
#include "gmglfunc.h"
#include "gmdatacore/object.h"
#include "gmgltexture.h"
#include "gmengine/gmgameobject.h"
#include "gmengine/gmgameworld.h"
#include "gmengine/gmcharacter.h"
#include "gmglobjectpainter.h"
#include "renders/gmgl_render.h"
#include "renders/gmgl_renders_object.h"
#include "renders/gmgl_renders_sky.h"
#include "renders/gmgl_renders_glyph.h"
#include "gmglgraphic_engine_default_shaders.h"
#include "foundation/gamemachine.h"
#include "foundation/gmconfig.h"

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

void GMGLGraphicEngine::start()
{
	installShaders();
}

void GMGLGraphicEngine::setCurrentWorld(GMGameWorld* world)
{
	D(d);
	d->world = world;
}

void GMGLGraphicEngine::newFrame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GMGLGraphicEngine::setViewport(const GMRect& rect)
{
	glViewport(rect.x, rect.y, rect.width, rect.height);
}

void GMGLGraphicEngine::drawObject(GMGameObject* object)
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

void GMGLGraphicEngine::drawObjectOnce(GMGameObject* object)
{
	D(d);
	GMfloat transform[16];
	object->getTransform().toArray(transform);
	GMGLShaderProgram* lastShaders = nullptr;
	object->onBeforeDraw();
	Object* coreObj = object->getObject();
	coreObj->getPainter()->draw(transform);
}

void GMGLGraphicEngine::installShaders()
{
	D(d);
	// 装载所有OpenGL着色器
	const GMString shaderMap[] =
	{
		_L("object"),
		_L("sky"),
		_L("glyph"),
	};

	// 按照Object顺序创建renders
	IRender* renders[] = {
		new GMGLRenders_Object(),
		new GMGLRenders_Sky(),
		new GMGLRenders_Glyph(),
	};

	GMGamePackage* package = GameMachine::instance().getGamePackageManager();

	for (GMint i = (GMint)GMMeshType::MeshTypeBegin; i < (GMint) GMMeshType::MeshTypeEnd; i++)
	{
		GMGLShaderProgram* shaderProgram = new GMGLShaderProgram();
		if (!d->shaderLoadCallback || (d->shaderLoadCallback && !d->shaderLoadCallback->onLoadShader((GMMeshType) i, shaderProgram)) )
		{
			if (!loadDefaultShaders((GMMeshType) i, shaderProgram))
			{
				delete shaderProgram;
				shaderProgram = nullptr;
			}
		}

		if (shaderProgram)
		{
			shaderProgram->load();
			registerShader((GMMeshType)i, shaderProgram);
		}

		registerRender((GMMeshType)i, renders[i]);
	}
}

bool GMGLGraphicEngine::loadDefaultShaders(const GMMeshType type, GMGLShaderProgram* shaderProgram)
{
	bool flag = false;
	switch (type)
	{
	case GMMeshType::Normal:
		shaderProgram->attachShader({ GL_VERTEX_SHADER, gmgl_shaders::object.vert });
		shaderProgram->attachShader({ GL_FRAGMENT_SHADER, gmgl_shaders::object.frag });
		flag = true;
		break;
	case GMMeshType::Sky:
		shaderProgram->attachShader({ GL_VERTEX_SHADER, gmgl_shaders::sky.vert });
		shaderProgram->attachShader({ GL_FRAGMENT_SHADER, gmgl_shaders::sky.frag });
		flag = true;
		break;
	case GMMeshType::Glyph:
		shaderProgram->attachShader({ GL_VERTEX_SHADER, gmgl_shaders::glyph.vert });
		shaderProgram->attachShader({ GL_FRAGMENT_SHADER, gmgl_shaders::glyph.frag });
		flag = true;
		break;
	default:
		break;
	}
	return flag;
}

void GMGLGraphicEngine::updateCameraView(const CameraLookAt& lookAt)
{
	D(d);
	updateMatrices(lookAt);

	BEGIN_ENUM(i, GMMeshType::MeshTypeBegin, GMMeshType::MeshTypeEnd)
	{
		IRender* render = getRender(i);
		GMMesh dummy;
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
	GMCharacter* character = getWorld()->getMajorCharacter();

	//TODO 
	d->projectionMatrix = character->getFrustum().getPerspective();
	d->viewMatrix = getViewMatrix(lookAt);

	character->getFrustum().updateViewMatrix(d->viewMatrix, d->projectionMatrix);
	character->getFrustum().update();
}

GMGameWorld* GMGLGraphicEngine::getWorld()
{
	D(d);
	return d->world;
}

void GMGLGraphicEngine::registerShader(GMMeshType objectType, AUTORELEASE GMGLShaderProgram* shaders)
{
	D(d);
	d->allShaders[objectType] = shaders;
}

GMGLShaderProgram* GMGLGraphicEngine::getShaders(GMMeshType objectType)
{
	D(d);
	if (d->allShaders.find(objectType) == d->allShaders.end())
		return nullptr;
	
	return d->allShaders[objectType];
}

void GMGLGraphicEngine::registerRender(GMMeshType objectType, AUTORELEASE IRender* render)
{
	D(d);
	d->allRenders[objectType] = render;
}

IRender* GMGLGraphicEngine::getRender(GMMeshType objectType)
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