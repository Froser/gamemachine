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
	disposeDeferredRenderQuad();

	for (auto iter = d->forwardRenderingShaders.begin(); iter != d->forwardRenderingShaders.end(); iter++)
	{
		if ((*iter).second)
			delete (*iter).second;
	}

	for (auto iter = d->deferredGeometryPassShader.begin(); iter != d->deferredGeometryPassShader.end(); iter++)
	{
		if ((*iter).second)
			delete (*iter).second;
	}

	if (d->deferredLightPassShader)
		delete d->deferredLightPassShader;

	for (auto iter = d->allRenders.begin(); iter != d->allRenders.end(); iter++)
	{
		if ((*iter).second)
			delete (*iter).second;
	}

	if (d->lightPassRender)
		delete d->lightPassRender;
}

void GMGLGraphicEngine::start()
{
	installShaders();
	glDepthFunc(GL_LEQUAL);
}

void GMGLGraphicEngine::newFrame()
{
	D(d);
	if (d->renderMode == GMGLRenderMode::DeferredRendering)
	{
		d->gbuffer.releaseBind();
		newFrameOnCurrentContext();
	}
	else
	{
		newFrameOnCurrentContext();
	}
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

		if (d->renderMode == GMGLRenderMode::DeferredRendering)
		{
			if (!refreshGBuffer())
			{
				gm_error("init gbuffer error");
				setRenderMode(GMGLRenderMode::ForwardRendering); // if error occurs, back into forward rendering
			}
		}
		break;
	}
	default:
		break;
	}
}

void GMGLGraphicEngine::drawObjects(GMGameObject *objects[], GMuint count)
{
	D(d);
	
	if (d->renderMode == GMGLRenderMode::ForwardRendering)
	{
		refreshForwardRenderLights();
		forwardRender(objects, count);
	}
	else
	{
		ASSERT(d->renderMode == GMGLRenderMode::DeferredRendering);
		geometryPass(objects, count);
		lightPass(objects, count);
	}
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
		GMGLShaderProgram* forwardShaderProgram = new GMGLShaderProgram();
		if (!d->shaderLoadCallback || (d->shaderLoadCallback && !d->shaderLoadCallback->onLoadForwardShader(type, *forwardShaderProgram)) )
		{
			if (!loadDefaultForwardShader(type, forwardShaderProgram))
			{
				delete forwardShaderProgram;
				forwardShaderProgram = nullptr;
			}
		}

		GMGLShaderProgram* deferredShaderGeometryPassProgram = new GMGLShaderProgram();
		if (!d->shaderLoadCallback || (d->shaderLoadCallback && !d->shaderLoadCallback->onLoadDeferredGeometryPassShader(type, *deferredShaderGeometryPassProgram)))
		{
			if (!loadDefaultDeferredGeometryPassShader(type, deferredShaderGeometryPassProgram))
			{
				delete deferredShaderGeometryPassProgram;
				deferredShaderGeometryPassProgram = nullptr;
			}
		}

		if (forwardShaderProgram)
			forwardShaderProgram->load();

		if (deferredShaderGeometryPassProgram)
			deferredShaderGeometryPassProgram->load();

		registerForwardRenderingShader(type, forwardShaderProgram);
		registerGeometryPassShader(type, deferredShaderGeometryPassProgram);
		registerRender(type, renders[(GMint) type]);
	}

	{
		GMGLShaderProgram* deferredShaderLightPassProgram = new GMGLShaderProgram();
		if (!d->shaderLoadCallback || (d->shaderLoadCallback && !d->shaderLoadCallback->onLoadDeferredLightPassShader(*deferredShaderLightPassProgram)))
		{
			if (!loadDefaultDeferredLightPassShader(deferredShaderLightPassProgram))
			{
				delete deferredShaderLightPassProgram;
				deferredShaderLightPassProgram = nullptr;
			}
		}

		if (deferredShaderLightPassProgram)
			deferredShaderLightPassProgram->load();

		registerLightPassShader(deferredShaderLightPassProgram);
	}

	d->lightPassRender = new GMGLRenders_LightPass();
}

bool GMGLGraphicEngine::loadDefaultForwardShader(const GMMeshType type, GMGLShaderProgram* shaderProgram)
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

bool GMGLGraphicEngine::loadDefaultDeferredGeometryPassShader(const GMMeshType type, GMGLShaderProgram* shaderProgram)
{
	bool flag = false;
	switch (type)
	{
	case GMMeshType::Model:
		shaderProgram->attachShader({ GL_VERTEX_SHADER, gmgl_shaders::object_pass.vert });
		shaderProgram->attachShader({ GL_FRAGMENT_SHADER, gmgl_shaders::object_pass.frag });
		flag = true;
		break;
	default:
		break;
	}
	return flag;
}

bool GMGLGraphicEngine::loadDefaultDeferredLightPassShader(GMGLShaderProgram* shaderProgram)
{
	return false;
}

void GMGLGraphicEngine::activateForwardRenderLight(const Vector<GMLight>& lights)
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

void GMGLGraphicEngine::activateLightPassLight(const Vector<GMLight>& lights)
{
	D(d);
	IRender* render = d->lightPassRender;
	GMint lightId[(GMuint)GMLightType::COUNT] = { 0 };

	for (auto& light : lights)
	{
		GMint id = lightId[(GMuint)light.getType()]++;
		render->activateLight(light, id);
	}
}

bool GMGLGraphicEngine::refreshGBuffer()
{
	D(d);
	GMRect rect = GM.getMainWindow()->getClientRect();
	d->gbuffer.dispose();
	return d->gbuffer.init(rect.width, rect.height);
}

void GMGLGraphicEngine::forwardRender(GMGameObject *objects[], GMuint count)
{
	D(d);
	for (GMuint i = 0; i < count; i++)
	{
		objects[i]->draw();
	}
}

void GMGLGraphicEngine::geometryPass(GMGameObject *objects[], GMuint count)
{
	D(d);
	d->gbuffer.newFrame();
	d->gbuffer.bindForWriting();

	for (GMuint i = 0; i < count; i++)
	{
		objects[i]->draw();
	}

	ASSERT(glGetError() == GL_NO_ERROR);
	d->gbuffer.releaseBind();
}

void GMGLGraphicEngine::lightPass(GMGameObject *objects[], GMuint count)
{
	D(d);
	newFrame();

#if 1
	d->deferredLightPassShader->useProgram();
	refreshDeferredRenderLights();
	d->gbuffer.activateTextures(d->deferredLightPassShader);
	renderDeferredRenderQuad();
#else
	// 开始写4个缓存
	const GMuint& w = d->gbuffer.getWidth(),
		&h = d->gbuffer.getHeight();
	GMuint hw = w / 2, hh = h / 2;

	GLenum errCode;
	d->gbuffer.bindForReading();
	d->gbuffer.setReadBuffer(GBufferTextureType::Position);
	glBlitFramebuffer(0, 0, w, h, 0, 0, hw, hh, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	ASSERT((errCode = glGetError()) == GL_NO_ERROR);

	d->gbuffer.setReadBuffer(GBufferTextureType::Bitangent);
	glBlitFramebuffer(0, 0, w, h, 0, hh, hw, h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	ASSERT((errCode = glGetError()) == GL_NO_ERROR);

	d->gbuffer.setReadBuffer(GBufferTextureType::DiffuseTexture);
	glBlitFramebuffer(0, 0, w, h, hw, hh, w, h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	ASSERT((errCode = glGetError()) == GL_NO_ERROR);

	d->gbuffer.setReadBuffer(GBufferTextureType::NormalMap);
	glBlitFramebuffer(0, 0, w, h, hw, 0, w, hh, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	ASSERT((errCode = glGetError()) == GL_NO_ERROR);
#endif
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

void GMGLGraphicEngine::refreshForwardRenderLights()
{
	D(d);
	if (d->needRefreshLights)
	{
		d->needRefreshLights = false;
		IGraphicEngine* engine = GM.getGraphicEngine();
		activateForwardRenderLight(d->lights);
	}
}

void GMGLGraphicEngine::refreshDeferredRenderLights()
{
	D(d);
	if (d->needRefreshLights)
	{
		d->needRefreshLights = false;
		IGraphicEngine* engine = GM.getGraphicEngine();
		activateLightPassLight(d->lights);
	}
}

void GMGLGraphicEngine::renderDeferredRenderQuad()
{
	D(d);
	if (d->quadVAO == 0)
	{
		GLfloat quadVertices[] = {
			// Positions        // Texture Coords
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};

		glGenVertexArrays(1, &d->quadVAO);
		glGenBuffers(1, &d->quadVBO);
		glBindVertexArray(d->quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, d->quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	glBindVertexArray(d->quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	GLenum errCode;
	ASSERT((errCode = glGetError()) == GL_NO_ERROR);
}

void GMGLGraphicEngine::disposeDeferredRenderQuad()
{
	D(d);
	glBindVertexArray(0);
	if (d->quadVAO)
		glDeleteVertexArrays(1, &d->quadVAO);

	if (d->quadVBO)
		glDeleteBuffers(1, &d->quadVBO);
}

void GMGLGraphicEngine::setViewport(const GMRect& rect)
{
	glViewport(rect.x, rect.y, rect.width, rect.height);
}

void GMGLGraphicEngine::registerForwardRenderingShader(GMMeshType objectType, AUTORELEASE GMGLShaderProgram* forwardShaderProgram)
{
	D(d);
	d->forwardRenderingShaders[objectType] = forwardShaderProgram;
}

void GMGLGraphicEngine::registerGeometryPassShader(GMMeshType objectType, AUTORELEASE GMGLShaderProgram* deferredGeometryPassProgram)
{
	D(d);
	d->deferredGeometryPassShader[objectType] = deferredGeometryPassProgram;
}

void GMGLGraphicEngine::registerLightPassShader(AUTORELEASE GMGLShaderProgram* deferredLightPassProgram)
{
	D(d);
	d->deferredLightPassShader = deferredLightPassProgram;
}

GMGLShaderProgram* GMGLGraphicEngine::getShaders(GMMeshType objectType)
{
	D(d);
	GMGLShaderProgram* prog;
	if (d->renderMode == GMGLRenderMode::ForwardRendering)
		prog = d->forwardRenderingShaders[objectType];
	else
		prog = d->deferredGeometryPassShader[objectType];

	if (!prog)
	{
		// TODO
		gm_warning("cannot find deferred shader of type %i, use forward shader instead.", objectType);
		return d->forwardRenderingShaders[objectType];
	}

	return prog;
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
	if (d->renderMode != mode)
		d->needRefreshLights = true;
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

void GMGLGraphicEngine::newFrameOnCurrentContext()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}