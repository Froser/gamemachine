#include "stdafx.h"
#include "shader_constants.h"
#include <algorithm>
#include "gmglgraphic_engine.h"
#include "gmdatacore/gmmodel.h"
#include "gmgltexture.h"
#include "gmglmodelpainter.h"
#include "renders/gmgl_renders_object.h"
#include "renders/gmgl_renders_glyph.h"
#include "renders/gmgl_renders_particle.h"
#include "foundation/gamemachine.h"
#include "foundation/gmstates.h"
#include "foundation/gmprofile.h"

extern "C"
{
	GLenum s_glErrCode;
}

class GMEffectRenderer
{
public:
	GMEffectRenderer(GMGLFramebuffer& effectBuffer, GMGLShaderProgram* program)
		: m_effectBuffer(effectBuffer)
		, m_program(program)
	{
		if (!m_effectBuffer.hasBegun()) // 防止绘制嵌套
		{
			m_isHost = true;
			m_effectBuffer.beginDrawEffects();
		}
	}

	~GMEffectRenderer()
	{
		if (m_isHost || !m_effectBuffer.hasBegun())
		{
			m_effectBuffer.endDrawEffects();
			m_effectBuffer.draw(m_program);
		}
	}

	GLuint framebuffer()
	{
		return m_effectBuffer.framebuffer();
	}

private:
	GMGLFramebuffer& m_effectBuffer;
	GMGLShaderProgram* m_program;
	bool m_isHost = false;
};

GMGLGraphicEngine::~GMGLGraphicEngine()
{
	D(d);
	disposeDeferredRenderQuad();

	for (auto iter : d->forwardRenderingShaders)
	{
		if (iter.second)
			delete iter.second;
	}

	for (auto iter : d->deferredCommonPassShaders)
	{
		if (iter.second)
			delete iter.second;
	}

	if (d->deferredLightPassShader)
		delete d->deferredLightPassShader;

	for (auto iter : d->allRenders)
	{
		if (iter.second)
			delete iter.second;
	}

	if (d->lightPassRender)
		delete d->lightPassRender;

	if (d->effectsShader)
		delete d->effectsShader;
}

void GMGLGraphicEngine::start()
{
	installShaders();
	glDepthFunc(GL_LEQUAL);
	createDeferredRenderQuad();
}

void GMGLGraphicEngine::newFrame()
{
	D(d);
	if (GMGetRenderState(RENDER_MODE) == GMStates_RenderOptions::DEFERRED)
	{
		d->gbuffer.releaseBind();
		newFrameOnCurrentContext();
	}
	else
	{
		newFrameOnCurrentContext();
	}
}

bool GMGLGraphicEngine::event(const GameMachineMessage& e)
{
	D(d);
	switch (e.msgType)
	{
	case GameMachineMessageType::WindowSizeChanged:
	{
		GMRect rect = GM.getMainWindow()->getClientRect();
		setViewport(rect);

		if (GMGetRenderState(RENDER_MODE) == GMStates_RenderOptions::DEFERRED)
		{
			if (!refreshGBuffer())
			{
				gm_error("init gbuffer error");
				GMSetRenderState(RENDER_MODE, GMStates_RenderOptions::FORWARD); // if error occurs, back into forward rendering
			}
		}

		if (!refreshFramebuffer())
			gm_error("init framebuffer error");
		break;
	}
	default:
		return false;
	}
	return true;
}

void GMGLGraphicEngine::drawObjects(GMGameObject *objects[], GMuint count)
{
	D(d);
	GM_PROFILE(drawObjects);
	GMRenderMode renderMode = GMGetRenderState(RENDER_MODE);
	if (renderMode != d->renderMode)
	{
		d->needRefreshLights = true;
		d->renderMode = renderMode;
	}

	if (renderMode == GMStates_RenderOptions::FORWARD)
	{
		refreshForwardRenderLights();

		{
			GMEffectRenderer effectRender(d->effectBuffer, d->effectsShader);
			forwardRender(objects, count);
		}
	}
	else
	{
		GM_ASSERT(renderMode == GMStates_RenderOptions::DEFERRED);
		// 把渲染图形分为两组，可延迟渲染组和不可延迟渲染组，先渲染可延迟渲染的图形
		groupGameObjects(objects, count);

		d->gbuffer.adjustViewport();
		geometryPass(d->deferredRenderingGameObjects);

		{
			GMEffectRenderer effectRender(d->effectBuffer, d->effectsShader);

			lightPass();
			d->gbuffer.copyDepthBuffer(effectRender.framebuffer());
			GMSetRenderState(RENDER_MODE, GMStates_RenderOptions::FORWARD);
			drawObjects(d->forwardRenderingGameObjects.data(), d->forwardRenderingGameObjects.size());
			GMSetRenderState(RENDER_MODE, GMStates_RenderOptions::DEFERRED);
		}

		viewFrameBuffer();
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
	if (!d->shaderLoadCallback)
	{
		gm_error("You must specify a IShaderLoadCallback");
		GM_ASSERT(false);
		return;
	}

	GM_FOREACH_ENUM_CLASS(type, GMMeshType::MeshTypeBegin, GMMeshType::MeshTypeEnd)
	{
		GMGLShaderProgram* forwardShaderProgram = new GMGLShaderProgram();
		d->shaderLoadCallback->onLoadForwardShader(type, *forwardShaderProgram);
		if (forwardShaderProgram)
			forwardShaderProgram->load();

		registerForwardRenderingShader(type, forwardShaderProgram);
		registerRender(type, renders[(GMint) type]);
	}

	{
		GMGLShaderProgram* deferredShaderLightPassProgram = new GMGLShaderProgram();
		d->shaderLoadCallback->onLoadDeferredLightPassShader(*deferredShaderLightPassProgram);
		deferredShaderLightPassProgram->load();
		registerLightPassShader(deferredShaderLightPassProgram);
	}

	{
		GMGLShaderProgram* effectsProgram = new GMGLShaderProgram();
		d->shaderLoadCallback->onLoadEffectsShader(*effectsProgram);
		effectsProgram->load();
		registerEffectsShader(effectsProgram);
	}

	GM_FOREACH_ENUM_CLASS(state, GMGLDeferredRenderState::PassingGeometry, GMGLDeferredRenderState::EndOfRenderState)
	{
		GMGLShaderProgram* shaderProgram = new GMGLShaderProgram();
		d->shaderLoadCallback->onLoadDeferredPassShader(state, *shaderProgram);
		shaderProgram->load();
		registerCommonPassShader(state, shaderProgram);
	}

	d->lightPassRender = new GMGLRenders_LightPass();
}

void GMGLGraphicEngine::activateForwardRenderLight(const Vector<GMLight>& lights)
{
	D(d);
	GM_FOREACH_ENUM(type, GMMeshType::MeshTypeBegin, GMMeshType::MeshTypeEnd)
	{
		IRender* render = getRender(type);
		render->activateLights(lights.data(), lights.size());
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
		render->activateLights(lights.data(), lights.size());
	}
}

bool GMGLGraphicEngine::refreshGBuffer()
{
	D(d);
	GMRect rect = GM.getMainWindow()->getClientRect();
	if (rect.width <= 0 || rect.height <= 0)
		return true;

	d->gbuffer.dispose();
	return d->gbuffer.init(rect);
}

bool GMGLGraphicEngine::refreshFramebuffer()
{
	D(d);
	GMRect rect = GM.getMainWindow()->getClientRect();
	if (rect.width <= 0 || rect.height <= 0)
		return true;

	d->effectBuffer.dispose();
	return d->effectBuffer.init(rect);
}

void GMGLGraphicEngine::forwardRender(GMGameObject *objects[], GMuint count)
{
	D(d);
	for (GMuint i = 0; i < count; i++)
	{
		objects[i]->draw();
	}
}

void GMGLGraphicEngine::geometryPass(Vector<GMGameObject*>& objects)
{
	D(d);
	d->gbuffer.beginPass();
	do
	{
		d->gbuffer.newFrame();
		d->gbuffer.bindForWriting();
		GM_CHECK_GL_ERROR();

		for (auto object : objects)
		{
			object->draw();
			GM_CHECK_GL_ERROR();
		}
		d->gbuffer.releaseBind();
		GM_CHECK_GL_ERROR();
	} while (d->gbuffer.nextPass());
}

void GMGLGraphicEngine::lightPass()
{
	D(d);
	d->deferredLightPassShader->useProgram();
	refreshDeferredRenderLights();
	d->gbuffer.activateTextures(d->deferredLightPassShader);
	renderDeferredRenderQuad();
}

void GMGLGraphicEngine::updateVPMatrices(const CameraLookAt& lookAt)
{
	D(d);
	GMMesh dummy;
	GM_FOREACH_ENUM(i, GMMeshType::MeshTypeBegin, GMMeshType::MeshTypeEnd)
	{
		IRender* render = getRender(i);
		dummy.setType(i);

		render->begin(this, &dummy, nullptr);
		render->updateVPMatrices(d->projectionMatrix, d->viewMatrix, lookAt);
		render->end();
	}
}

void GMGLGraphicEngine::groupGameObjects(GMGameObject *objects[], GMuint count)
{
	D(d);
	d->deferredRenderingGameObjects.clear();
	d->deferredRenderingGameObjects.reserve(count);
	d->forwardRenderingGameObjects.clear();
	d->forwardRenderingGameObjects.reserve(count);

	for (GMuint i = 0; i < count; i++)
	{
		if (objects[i]->canDeferredRendering())
			d->deferredRenderingGameObjects.push_back(objects[i]);
		else
			d->forwardRenderingGameObjects.push_back(objects[i]);
	}
}

void GMGLGraphicEngine::viewFrameBuffer()
{
	D(d);
	GMint fbIdx = GMGetDebugState(FRAMEBUFFER_VIEWER_INDEX);
	if (fbIdx > 0)
	{
		glDisable(GL_DEPTH_TEST);
		GMint x = GMGetDebugState(FRAMEBUFFER_VIEWER_X),
			y = GMGetDebugState(FRAMEBUFFER_VIEWER_Y),
			width = GMGetDebugState(FRAMEBUFFER_VIEWER_WIDTH),
			height = GMGetDebugState(FRAMEBUFFER_VIEWER_HEIGHT);
		d->gbuffer.beginPass();
		d->gbuffer.bindForReading();
		d->gbuffer.setReadBuffer((GBufferGeometryType)(fbIdx - 1));
		glBlitFramebuffer(0, 0, d->gbuffer.getWidth(), d->gbuffer.getHeight(), x, y, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		GM_CHECK_GL_ERROR();
		d->gbuffer.releaseBind();
	}
}

void GMGLGraphicEngine::updateCameraView(const CameraLookAt& lookAt)
{
	D(d);
	GM_BEGIN_CHECK_GL_ERROR
	updateMatrices(lookAt);

	// 遍历每一种着色器
	GMRenderMode renderModeCache = GMGetRenderState(RENDER_MODE);
	auto renderStateCache = getRenderState();

	GMSetRenderState(RENDER_MODE, GMStates_RenderOptions::FORWARD);
	updateVPMatrices(lookAt);

	// 更新material pass著色器
	GMSetRenderState(RENDER_MODE, GMStates_RenderOptions::DEFERRED);
	GM_FOREACH_ENUM_CLASS(state, GMGLDeferredRenderState::PassingGeometry, GMGLDeferredRenderState::EndOfRenderState)
	{
		setRenderState(state);
		updateVPMatrices(lookAt);
	}
	setRenderState(renderStateCache);
	GMSetRenderState(RENDER_MODE, renderModeCache);

	if (renderModeCache == GMStates_RenderOptions::DEFERRED)
	{
		// 更新light pass著色器
		GMMesh dummy;
		IRender* render = d->lightPassRender;
		render->begin(this, &dummy, nullptr);
		render->updateVPMatrices(d->projectionMatrix, d->viewMatrix, lookAt);
		render->end();
	}
	GM_END_CHECK_GL_ERROR
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

void GMGLGraphicEngine::createDeferredRenderQuad()
{
	D(d);
	if (d->quadVAO == 0)
	{
		static GLfloat quadVertices[] = {
			// Positions		// Texture Coords
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};

		glGenVertexArrays(1, &d->quadVAO);
		glBindVertexArray(d->quadVAO);
		glGenBuffers(1, &d->quadVBO);
		glBindBuffer(GL_ARRAY_BUFFER, d->quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
}

void GMGLGraphicEngine::renderDeferredRenderQuad()
{
	D(d);
	glDisable(GL_CULL_FACE);
	glBindVertexArray(d->quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	GM_CHECK_GL_ERROR();
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
	GM_BEGIN_CHECK_GL_ERROR
	glViewport(rect.x, rect.y, rect.width, rect.height);
	GM_END_CHECK_GL_ERROR
}

void GMGLGraphicEngine::registerForwardRenderingShader(GMMeshType objectType, AUTORELEASE GMGLShaderProgram* forwardShaderProgram)
{
	D(d);
	d->forwardRenderingShaders[objectType] = forwardShaderProgram;
}

void GMGLGraphicEngine::registerCommonPassShader(GMGLDeferredRenderState state, AUTORELEASE GMGLShaderProgram* shaderProgram)
{
	D(d);
	d->deferredCommonPassShaders[state] = shaderProgram;
}

void GMGLGraphicEngine::registerLightPassShader(AUTORELEASE GMGLShaderProgram* deferredLightPassProgram)
{
	D(d);
	d->deferredLightPassShader = deferredLightPassProgram;
}

void GMGLGraphicEngine::registerEffectsShader(AUTORELEASE GMGLShaderProgram* effectsShader)
{
	D(d);
	d->effectsShader = effectsShader;
}

GMGLShaderProgram* GMGLGraphicEngine::getShaders(GMMeshType objectType)
{
	D(d);
	GMGLShaderProgram* prog;
	GMRenderMode renderMode = GMGetRenderState(RENDER_MODE);
	if (renderMode == GMStates_RenderOptions::FORWARD)
	{
		prog = d->forwardRenderingShaders[objectType];
	}
	else
	{
		GM_ASSERT(renderMode == GMStates_RenderOptions::DEFERRED);
		prog = d->deferredCommonPassShaders[d->renderState];
	}

	GM_ASSERT(prog);
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

void GMGLGraphicEngine::addLight(const GMLight& light)
{
	D(d);
	d->lights.push_back(light);
	d->needRefreshLights = true;
}

void GMGLGraphicEngine::beginCreateStencil()
{
	D(d);
	d->stencilRenderModeCache = GMGetRenderState(RENDER_MODE);
	GMSetRenderState(RENDER_MODE, GMStates_RenderOptions::FORWARD);
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
	GMSetRenderState(RENDER_MODE, d->stencilRenderModeCache);
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

void GMGLGraphicEngine::beginBlend()
{
	D(d);
	d->renderModeForBlend = GMGetRenderState(RENDER_MODE);
	d->isBlending = true;
	GMSetRenderState(RENDER_MODE, GMStates_RenderOptions::FORWARD);
}

void GMGLGraphicEngine::endBlend()
{
	D(d);
	GMSetRenderState(RENDER_MODE, d->renderModeForBlend);
	d->isBlending = false;
}

void GMGLGraphicEngine::beginFullRendering()
{
	D(d);
	d->effectBuffer.setUseFullscreenFramebuffer(true);
}

void GMGLGraphicEngine::endFullRendering()
{
	D(d);
	d->effectBuffer.setUseFullscreenFramebuffer(false);
}

void GMGLGraphicEngine::newFrameOnCurrentContext()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}