#include "stdafx.h"
#include "shader_constants.h"
#include <algorithm>
#include "gmglgraphic_engine.h"
#include "gmdatacore/object.h"
#include "gmgltexture.h"
#include "gmengine/gmgameobject.h"
#include "gmglobjectpainter.h"
#include "renders/gmgl_renders_object.h"
#include "renders/gmgl_renders_glyph.h"
#include "gmglgraphic_engine_default_shaders.h"
#include "foundation/gamemachine.h"
#include "foundation/gmconfig.h"

GMGLGraphicEngine::GMGLGraphicEngine()
{
	D(d);
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
	glDepthFunc(GL_LEQUAL);
	drawObjectOnce(object);
}

void GMGLGraphicEngine::drawObjectOnce(GMGameObject* object)
{
	D(d);
	GMfloat transform[16];
	object->getTransform().toArray(transform);
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
		_L("glyph"),
	};

	// 按照Object顺序创建renders
	IRender* renders[] = {
		new GMGLRenders_Object(),
		new GMGLRenders_Glyph(),
	};

	GMGamePackage* package = GameMachine::instance().getGamePackageManager();

	GM_FOREACH_ENUM_CLASS(type, GMMeshType, GMMeshType::MeshTypeBegin, GMMeshType::MeshTypeEnd)
	{
		GMGLShaderProgram* shaderProgram = new GMGLShaderProgram();
		if (!d->shaderLoadCallback || (d->shaderLoadCallback && !d->shaderLoadCallback->onLoadShader(type, shaderProgram)) )
		{
			if (!loadDefaultShaders(type, shaderProgram))
			{
				delete shaderProgram;
				shaderProgram = nullptr;
			}
		}

		if (shaderProgram)
		{
			shaderProgram->load();
			registerShader(type, shaderProgram);
		}

		registerRender(type, renders[(GMint) type]);
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

	GM_FOREACH_ENUM(i, GMMeshType::MeshTypeBegin, GMMeshType::MeshTypeEnd)
	{
		IRender* render = getRender(i);
		GMMesh dummy;
		dummy.setType(i);

		render->begin(this, &dummy, nullptr);
		render->updateVPMatrices(d->projectionMatrix, d->viewMatrix, lookAt);
		render->end();
	}
}

void GMGLGraphicEngine::updateMatrices(const CameraLookAt& lookAt)
{
	D(d);
	GMCamera& camera = GameMachine::instance().getCamera();

	d->projectionMatrix = camera.getFrustum().getPerspective();
	d->viewMatrix = getViewMatrix(lookAt);

	camera.getFrustum().updateViewMatrix(d->viewMatrix, d->projectionMatrix);
	camera.getFrustum().update();
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

Vector<GMLight> GMGLGraphicEngine::getLights()
{
	D(d);
	return d->lights;
}

void GMGLGraphicEngine::addLight(const GMLight& light)
{
	D(d);
	d->lights.push_back(light);
}