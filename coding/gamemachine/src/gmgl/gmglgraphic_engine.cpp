#include "stdafx.h"
#include "shader_constants.h"
#include <algorithm>
#include "gmglgraphic_engine.h"
#include "gmdatacore/gmmodel.h"
#include "gmgltexture.h"
#include "gmengine/gmgameobject.h"
#include "gmglmodelpainter.h"
#include "renders/gmgl_renders_object.h"
#include "renders/gmgl_renders_glyph.h"
#include "renders/gmgl_renders_particle.h"
#include "gmglgraphic_engine_default_shaders.h"
#include "foundation/gamemachine.h"
#include "foundation/gmconfig.h"

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
	glDepthFunc(GL_LEQUAL);
}

void GMGLGraphicEngine::newFrame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void GMGLGraphicEngine::event(const GameMachineMessage& e)
{
	D(d);
	switch (e.msgType)
	{
	case GameMachineMessageType::OnWindowSizeChanged:
	{
		GMRect rect = GM.getMainWindow()->getClientRect();
		setViewport(rect);
		d->gbuffer.dispose();
		if (!d->gbuffer.init(rect.width, rect.height))
		{
			gm_error("init gbuffer error");
			setRenderMode(GMGLRenderMode::ForwardRendering); // if error occurs, back into foward rendering
		}
		break;
	}
	default:
		break;
	}
}

void GMGLGraphicEngine::drawObject(GMGameObject* object)
{
	D(d);

	// 光照一般只需要设置一次，除非光照被改变
	if (d->needRefreshLights)
	{
		d->needRefreshLights = false;
		IGraphicEngine* engine = GM.getGraphicEngine();
		activateLight(d->lights);
	}

	object->draw();
}

void GMGLGraphicEngine::installShaders()
{
	D(d);
	// 按照Object顺序创建renders
	IRender* renders[] = {
		new GMGLRenders_Object(),
		new GMGLRenders_Glyph(),
		new GMGLRenders_Particle(),
	};

	GMGamePackage* package = GM.getGamePackageManager();

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
	case GMMeshType::Model:
		shaderProgram->attachShader({ GL_VERTEX_SHADER, gmgl_shaders::object.vert });
		shaderProgram->attachShader({ GL_FRAGMENT_SHADER, gmgl_shaders::object.frag });
		flag = true;
		break;
	case GMMeshType::Glyph:
		shaderProgram->attachShader({ GL_VERTEX_SHADER, gmgl_shaders::glyph.vert });
		shaderProgram->attachShader({ GL_FRAGMENT_SHADER, gmgl_shaders::glyph.frag });
		flag = true;
		break;
	case GMMeshType::Particles:
		shaderProgram->attachShader({ GL_VERTEX_SHADER, gmgl_shaders::particles.vert });
		shaderProgram->attachShader({ GL_FRAGMENT_SHADER, gmgl_shaders::particles.frag });
		flag = true;
		break;
	default:
		break;
	}
	return flag;
}

void GMGLGraphicEngine::activateLight(const Vector<GMLight>& lights)
{
	D(d);
	GM_FOREACH_ENUM(type, GMMeshType::MeshTypeBegin, GMMeshType::MeshTypeEnd)
	{
		IRender* render = getRender(type);
		GMint lightId[(GMuint)GMLightType::COUNT] = { 0 };

		for (auto& light : lights)
		{
			GMint id = lightId[(GMuint)light.getType()]++;
			render->activateLight(light, id);
		}
	}
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
	GMCamera& camera = GM.getCamera();

	d->projectionMatrix = camera.getFrustum().getPerspective();
	d->viewMatrix = getViewMatrix(lookAt);

	camera.getFrustum().updateViewMatrix(d->viewMatrix, d->projectionMatrix);
	camera.getFrustum().update();
}

void GMGLGraphicEngine::setViewport(const GMRect& rect)
{
	glViewport(rect.x, rect.y, rect.width, rect.height);
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

void GMGLGraphicEngine::setRenderMode(GMGLRenderMode mode)
{
	D(d);
	d->renderMode = mode;
}

ResourceContainer* GMGLGraphicEngine::getResourceContainer()
{
	D(d);
	return &d->resourceContainer;
}

void GMGLGraphicEngine::addLight(const GMLight& light)
{
	D(d);
	d->lights.push_back(light);
	d->needRefreshLights = true;
}

void GMGLGraphicEngine::beginCreateStencil()
{
	D(d);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}

void GMGLGraphicEngine::endCreateStencil()
{
	D(d);
	glStencilMask(0x00);
}

void GMGLGraphicEngine::beginUseStencil(bool inverse)
{
	if (!inverse)
		glStencilFunc(GL_EQUAL, 1, 0xFF);
	else
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
}

void GMGLGraphicEngine::endUseStencil()
{
	glDisable(GL_STENCIL_TEST);
}