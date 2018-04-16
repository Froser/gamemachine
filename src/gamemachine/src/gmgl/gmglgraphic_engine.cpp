#include "stdafx.h"
#include <GL/glew.h>
#include "shader_constants.h"
#include <algorithm>
#include "gmglgraphic_engine.h"
#include "gmdata/gmmodel.h"
#include "gmgltexture.h"
#include "gmglmodelpainter.h"
#include "gmglrenderers.h"
#include "foundation/gamemachine.h"
#include "foundation/gmconfigs.h"
#include "foundation/gmprofile.h"

extern "C"
{
	GLenum s_glErrCode;

	namespace
	{
		void GLAPIENTRY GL_MessageCallback(
			GLenum source,
			GLenum type,
			GLuint id,
			GLenum severity,
			GLsizei length,
			const GLchar* message,
			const void* userParam
			)
		{
			gm_error("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
				(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
				type, severity, message);
		}
	}
}

class GMEffectRenderer
{
public:
	GMEffectRenderer(GMGLFramebuffer& framebuffer, GMGLShaderProgram* program)
		: m_effectBuffer(framebuffer)
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

GMGLGraphicEngine::GMGLGraphicEngine()
{
	D(d);
	d->renderConfig = GM.getConfigs().getConfig(GMConfigs::Render).asRenderConfig();
	d->debugConfig = GM.getConfigs().getConfig(GMConfigs::Debug).asDebugConfig();
}

GMGLGraphicEngine::~GMGLGraphicEngine()
{
	D(d);
	disposeDeferredRenderQuad();
	GM_delete(d->effectsShaderProgram);
	GM_delete(d->forwardShaderProgram);
	GM_delete(d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER]);
	GM_delete(d->deferredShaderPrograms[DEFERRED_LIGHT_PASS_SHADER]);
}

void GMGLGraphicEngine::init()
{
	installShaders();
	glEnable(GL_MULTISAMPLE);

	auto& runningState = GM.getGameMachineRunningStates();
	glClearDepth(runningState.farZ);
	glDepthFunc(GL_LEQUAL);

	glClearColor(0, 0, 0, 1);
	glEnable(GL_STENCIL_TEST);
	glClearStencil(0);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	createDeferredRenderQuad();

#if _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback((GLDEBUGPROC)GL_MessageCallback, 0);
#endif
}

void GMGLGraphicEngine::newFrame()
{
	D(d);
	if (getCurrentRenderMode() == GMRenderMode::Deferred)
	{
		d->gbuffer.releaseBind();
		newFrameOnCurrentFramebuffer();
	}
	else
	{
		newFrameOnCurrentFramebuffer();
	}
}

bool GMGLGraphicEngine::event(const GameMachineMessage& e)
{
	D(d);
	switch (e.msgType)
	{
	case GameMachineMessageType::WindowSizeChanged:
	{
		const GMRect& rect = GM.getGameMachineRunningStates().clientRect;
		setViewport(rect);

		if (d->renderConfig.get(GMRenderConfigs::RenderMode).toEnum<GMRenderMode>() == GMRenderMode::Deferred)
		{
			if (!refreshGBuffer())
			{
				gm_error("init gbuffer error");
				d->renderConfig.set(GMRenderConfigs::RenderMode, (GMint)GMRenderMode::Forward);
			}
		}

		if (!refreshFramebuffer())
			gm_error("init framebuffer error");
		break;
	}
	}
	return false;
}

void GMGLGraphicEngine::drawObjects(GMGameObject *objects[], GMuint count, GMBufferMode bufferMode)
{
	D(d);
	GM_PROFILE("drawObjects");
	if (!count)
		return;

#if _DEBUG
	++d->drawingLevel;
#endif

	if (bufferMode == GMBufferMode::NoFramebuffer)
	{
		directDraw(objects, count);
	}
	else
	{
		GMRenderMode renderMode = d->renderConfig.get(GMRenderConfigs::RenderMode).toEnum<GMRenderMode>();
		if (renderMode != d->renderMode)
			d->needRefreshLights = true;
		setCurrentRenderMode(renderMode);

		if (renderMode == GMRenderMode::Forward)
		{
			forwardDraw(objects, count);
		}
		else
		{
			GM_ASSERT(renderMode == GMRenderMode::Deferred);
			// 把渲染图形分为两组，可延迟渲染组和不可延迟渲染组，先渲染可延迟渲染的图形
			groupGameObjects(objects, count);

			if (d->deferredRenderingGameObjects.empty())
			{
				forwardDraw(d->forwardRenderingGameObjects.data(), d->forwardRenderingGameObjects.size());
			}
			else
			{
				d->gbuffer.adjustViewport();
				geometryPass(d->deferredRenderingGameObjects);

				{
					GMEffectRenderer effectRender(d->framebuffer, d->effectsShaderProgram);
					lightPass();
					d->gbuffer.copyDepthBuffer(effectRender.framebuffer());
					forwardDraw(d->forwardRenderingGameObjects.data(), d->forwardRenderingGameObjects.size());
					setCurrentRenderMode(GMRenderMode::Deferred);
				}
			}

			viewGBufferFrameBuffer();
		}
	}

#if _DEBUG
	--d->drawingLevel;
#endif
}

void GMGLGraphicEngine::installShaders()
{
	D(d);
	if (!d->shaderLoadCallback)
	{
		gm_error("You must specify a IShaderLoadCallback");
		GM_ASSERT(false);
		return;
	}

	d->shaderLoadCallback->onLoadShaders(this);
}

bool GMGLGraphicEngine::setInterface(GameMachineInterfaceID id, void* in)
{
	D(d);
	switch (id)
	{
	case GameMachineInterfaceID::GLEffectShaderProgram:
		d->effectsShaderProgram = static_cast<GMGLShaderProgram*>(in);
		d->effectsShaderProgram->load();
		break;
	case GameMachineInterfaceID::GLForwardShaderProgram:
		d->forwardShaderProgram = static_cast<GMGLShaderProgram*>(in);
		d->forwardShaderProgram->load();
		break;
	case GameMachineInterfaceID::GLDeferredShaderGeometryProgram:
		d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER] = static_cast<GMGLShaderProgram*>(in);
		d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER]->load();
		break;
	case GameMachineInterfaceID::GLDeferredShaderLightProgram:
		d->deferredShaderPrograms[DEFERRED_LIGHT_PASS_SHADER] = static_cast<GMGLShaderProgram*>(in);
		d->deferredShaderPrograms[DEFERRED_LIGHT_PASS_SHADER]->load();
		break;
	default:
		return false;
	}
	return true;
}

void GMGLGraphicEngine::activateLights(const Vector<GMLight>& lights)
{
	D(d);
	updateShader();

	GMRenderMode renderMode = d->renderConfig.get(GMRenderConfigs::RenderMode).toEnum<GMRenderMode>();
	GMGLShaderProgram* progs[] = {
		d->forwardShaderProgram,
		renderMode== GMRenderMode::Forward ? nullptr : d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER],
		renderMode== GMRenderMode::Forward ? nullptr : d->deferredShaderPrograms[DEFERRED_LIGHT_PASS_SHADER]
	};

	for (GMint i = 0; i < GM_array_size(progs); ++i)
	{
		GMGLShaderProgram* prog = progs[i];
		if (!prog)
			continue;

		auto& svd = prog->getDesc();

		prog->useProgram();
		GMint lightId[(GMuint)GMLightType::COUNT] = { 0 };
		for (auto& light : lights)
		{
			GMint id = lightId[(GMuint)light.getType()]++;
			switch (light.getType())
			{
			case GMLightType::AMBIENT:
			{
				const char* uniform = getLightUniformName(svd, GMLightType::AMBIENT, id);
				char u_color[GMGL_MAX_UNIFORM_NAME_LEN];
				combineUniform(u_color, uniform, ".", svd.LightAttributes.Color);
				prog->setVec3(u_color, light.getLightColor());
				break;
			}
			case GMLightType::SPECULAR:
			{
				const char* uniform = getLightUniformName(svd, GMLightType::SPECULAR, id);
				char u_color[GMGL_MAX_UNIFORM_NAME_LEN], u_position[GMGL_MAX_UNIFORM_NAME_LEN];
				combineUniform(u_color, uniform, ".", svd.LightAttributes.Color);
				combineUniform(u_position, uniform, ".", svd.LightAttributes.Position);
				prog->setVec3(u_color, light.getLightColor());
				prog->setVec3(u_position, light.getLightPosition());
				break;
			}
			default:
				break;
			}
		}
		prog->setInt(svd.AmbientLight.Count, lightId[(GMint)GMLightType::AMBIENT]);
		prog->setInt(svd.SpecularLight.Count, lightId[(GMint)GMLightType::SPECULAR]);
	}
}

bool GMGLGraphicEngine::refreshGBuffer()
{
	D(d);
	const GMRect& rect = GM.getGameMachineRunningStates().clientRect;
	if (rect.width <= 0 || rect.height <= 0)
		return true;

	d->gbuffer.dispose();
	return d->gbuffer.init(rect);
}

bool GMGLGraphicEngine::refreshFramebuffer()
{
	D(d);
	const GMRect& rect = GM.getGameMachineRunningStates().clientRect;
	if (rect.width <= 0 || rect.height <= 0)
		return true;

	d->framebuffer.dispose();
	return d->framebuffer.init(rect);
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

		for (auto object : objects)
		{
			object->draw();
		}
		d->gbuffer.releaseBind();
	} while (d->gbuffer.nextPass());
}

void GMGLGraphicEngine::lightPass()
{
	D(d);
	GM_ASSERT(getRenderState() == GMGLDeferredRenderState::PassingLight);
	activateLightsIfNecessary();
	d->gbuffer.activateTextures();
	renderDeferredRenderQuad();
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

void GMGLGraphicEngine::viewGBufferFrameBuffer()
{
	D(d);
	GMint fbIdx = d->debugConfig.get(GMDebugConfigs::FrameBufferIndex_I32).toInt();
	if (fbIdx > 0)
	{
		glDisable(GL_DEPTH_TEST);
		GMint x = d->debugConfig.get(GMDebugConfigs::FrameBufferPositionX_I32).toInt(),
			y = d->debugConfig.get(GMDebugConfigs::FrameBufferPositionY_I32).toInt(),
			width = d->debugConfig.get(GMDebugConfigs::FrameBufferWidth_I32).toInt(),
			height = d->debugConfig.get(GMDebugConfigs::FrameBufferHeight_I32).toInt();

		GM_BEGIN_CHECK_GL_ERROR
		d->gbuffer.beginPass();
		GM_END_CHECK_GL_ERROR

		GM_BEGIN_CHECK_GL_ERROR
		d->gbuffer.bindForReading();
		GM_END_CHECK_GL_ERROR

		GM_BEGIN_CHECK_GL_ERROR
		d->gbuffer.setReadBuffer((GBufferGeometryType)(fbIdx - 1));
		GM_END_CHECK_GL_ERROR

		GM_BEGIN_CHECK_GL_ERROR
		glBlitFramebuffer(0, 0, d->gbuffer.getWidth(), d->gbuffer.getHeight(), x, y, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		GM_END_CHECK_GL_ERROR
		d->gbuffer.releaseBind();
	}
}

void GMGLGraphicEngine::update(GMUpdateDataType type)
{
	D(d);
	switch (type)
	{
	case GMUpdateDataType::ProjectionMatrix:
	{
		updateProjectionMatrix();
		break;
	}
	case GMUpdateDataType::ViewMatrix:
	{
		updateViewMatrix();
		break;
	}
	case GMUpdateDataType::TurnOffCubeMap:
	{
		GM_BEGIN_CHECK_GL_ERROR
		glActiveTexture(GL_TEXTURE0 + GMTextureRegisterQuery<GMTextureType::CubeMap>::Value);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		GM_END_CHECK_GL_ERROR
		break;
	}
	default:
		GM_ASSERT(false);
		break;
	}
}

void GMGLGraphicEngine::updateProjectionMatrix()
{
	D(d);
	auto& desc = d->forwardShaderProgram->getDesc();
	GMCamera& camera = GM.getCamera();
	const GMMat4& proj = camera.getFrustum().getProjectionMatrix();
	GM_BEGIN_CHECK_GL_ERROR
	d->forwardShaderProgram->useProgram();
	d->forwardShaderProgram->setMatrix4(desc.ProjectionMatrix, proj);
	GM_END_CHECK_GL_ERROR

	GM_BEGIN_CHECK_GL_ERROR
	d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER]->useProgram();
	d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER]->setMatrix4(desc.ProjectionMatrix, proj);
	d->deferredShaderPrograms[DEFERRED_LIGHT_PASS_SHADER]->useProgram();
	d->deferredShaderPrograms[DEFERRED_LIGHT_PASS_SHADER]->setMatrix4(desc.ProjectionMatrix, proj);
	GM_END_CHECK_GL_ERROR
}

void GMGLGraphicEngine::updateViewMatrix()
{
	D(d);
	auto& desc = d->forwardShaderProgram->getDesc();
	GMCamera& camera = GM.getCamera();
	const GMMat4& viewMatrix = camera.getFrustum().getViewMatrix();
	const GMCameraLookAt& lookAt = camera.getLookAt();
	GMFloat4 vec;
	lookAt.position.loadFloat4(vec);

	GM_BEGIN_CHECK_GL_ERROR
	// 视觉位置，用于计算光照
	d->forwardShaderProgram->useProgram();
	d->forwardShaderProgram->setVec4(desc.ViewPosition, vec);
	d->forwardShaderProgram->setMatrix4(desc.ViewMatrix, viewMatrix);
	d->forwardShaderProgram->setMatrix4(GMSHADER_INVERSE_VIEW_MATRIX, Inverse(viewMatrix));
	GM_END_CHECK_GL_ERROR

	GM_BEGIN_CHECK_GL_ERROR
	// 视觉位置，用于计算光照
	d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER]->useProgram();
	d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER]->setVec4(desc.ViewPosition, vec);
	d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER]->setMatrix4(desc.ViewMatrix, viewMatrix);
	d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER]->setMatrix4(GMSHADER_INVERSE_VIEW_MATRIX, Inverse(viewMatrix));
	d->deferredShaderPrograms[DEFERRED_LIGHT_PASS_SHADER]->useProgram();
	d->deferredShaderPrograms[DEFERRED_LIGHT_PASS_SHADER]->setVec4(desc.ViewPosition, vec);
	d->deferredShaderPrograms[DEFERRED_LIGHT_PASS_SHADER]->setMatrix4(desc.ViewMatrix, viewMatrix);
	d->deferredShaderPrograms[DEFERRED_LIGHT_PASS_SHADER]->setMatrix4(GMSHADER_INVERSE_VIEW_MATRIX, Inverse(viewMatrix));
	GM_END_CHECK_GL_ERROR
}

void GMGLGraphicEngine::directDraw(GMGameObject *objects[], GMuint count)
{
	D(d);
	setCurrentRenderMode(GMRenderMode::Forward);
	d->framebuffer.releaseBind();
	forwardRender(objects, count);
}

void GMGLGraphicEngine::forwardDraw(GMGameObject *objects[], GMuint count)
{
	D(d);
	setCurrentRenderMode(GMRenderMode::Forward);
	activateLightsIfNecessary();
	if (d->renderConfig.get(GMRenderConfigs::FilterMode).toEnum<GMFilterMode::Mode>() != GMFilterMode::None)
	{
		GMEffectRenderer effectRender(d->framebuffer, d->effectsShaderProgram);
		forwardRender(objects, count);
	}
	else
	{
		forwardRender(objects, count);
	}
}

void GMGLGraphicEngine::updateShader()
{
	D(d);
	GMRenderMode renderMode = getCurrentRenderMode();
	if (renderMode == GMRenderMode::Forward)
	{
		d->forwardShaderProgram->useProgram();
	}
	else
	{
		if (getRenderState() != GMGLDeferredRenderState::PassingLight)
		{
			d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER]->useProgram();
			GM_ASSERT(renderMode == GMRenderMode::Deferred);
			if (d->renderState == GMGLDeferredRenderState::PassingGeometry)
				d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER]->setInt(GMSHADER_SHADER_PROC, GMShaderProc::GEOMETRY_PASS);
			else if (d->renderState == GMGLDeferredRenderState::PassingMaterial)
				d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER]->setInt(GMSHADER_SHADER_PROC, GMShaderProc::MATERIAL_PASS);
			else
				GM_ASSERT(false);
		}
		else
		{
			d->deferredShaderPrograms[DEFERRED_LIGHT_PASS_SHADER]->useProgram();
		}
	}
}

void GMGLGraphicEngine::activateLightsIfNecessary()
{
	D(d);
	if (d->needRefreshLights)
	{
		d->needRefreshLights = false;
		IGraphicEngine* engine = GM.getGraphicEngine();
		activateLights(d->lights);
	}
}

void GMGLGraphicEngine::createDeferredRenderQuad()
{
	D(d);
	if (d->quadVAO == 0)
	{
		static GLfloat quadVertices[] = {
			// Positions		// Texture Coords
			-1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
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
	GM_BEGIN_CHECK_GL_ERROR
	glDisable(GL_CULL_FACE);
	glBindVertexArray(d->quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	GM_END_CHECK_GL_ERROR
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

IRenderer* GMGLGraphicEngine::getRenderer(GMModelType objectType)
{
	D(d);
	static GMGLRenderer_2D s_renderer2d;
	static GMGLRenderer_3D s_renderer3d;
	static GMGLRenderer_CubeMap s_rendererCubeMap;
	switch (objectType)
	{
	case GMModelType::Model2D:
	case GMModelType::Glyph:
	case GMModelType::Particles:
		return &s_renderer2d;
	case GMModelType::Model3D:
		return &s_renderer3d;
	case GMModelType::CubeMap:
		return &s_rendererCubeMap;
	default:
		GM_ASSERT(false);
		return nullptr;
	}
}

void GMGLGraphicEngine::addLight(const GMLight& light)
{
	D(d);
	d->lights.push_back(light);
	d->needRefreshLights = true;
}

void GMGLGraphicEngine::removeLights()
{
	D(d);
	d->lights.clear();
	d->needRefreshLights = true;
}

void GMGLGraphicEngine::clearStencil()
{
	clearStencilOnCurrentFramebuffer();
}

void GMGLGraphicEngine::beginBlend(GMS_BlendFunc sfactor, GMS_BlendFunc dfactor)
{
	D(d);
	GM_ASSERT(!d->isBlending); // 不能重复进行多次Blend
	d->renderModeForBlend = getCurrentRenderMode();
	d->isBlending = true;
	d->blendsfactor = sfactor;
	d->blenddfactor = dfactor;
	setCurrentRenderMode(GMRenderMode::Forward);
}

void GMGLGraphicEngine::endBlend()
{
	D(d);
	setCurrentRenderMode(d->renderModeForBlend);
	d->isBlending = false;
}

IShaderProgram* GMGLGraphicEngine::getShaderProgram(GMShaderProgramType type)
{
	D(d);
	if (type == GMShaderProgramType::CurrentShaderProgram)
	{
		if (getCurrentRenderMode() == GMRenderMode::Forward)
			return d->forwardShaderProgram;
		GM_ASSERT(getCurrentRenderMode() == GMRenderMode::Deferred);
		if (getRenderState() != GMGLDeferredRenderState::PassingLight)
			return d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER];
		return d->deferredShaderPrograms[DEFERRED_LIGHT_PASS_SHADER];
	}
	else if (type == GMShaderProgramType::ForwardShaderProgram)
	{
		return d->forwardShaderProgram;
	}
	else if (type == GMShaderProgramType::DeferredGeometryPassShaderProgram)
	{
		return d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER];
	}
	else
	{
		GM_ASSERT(type == GMShaderProgramType::DeferredLightPassShaderProgram);
		return d->deferredShaderPrograms[DEFERRED_LIGHT_PASS_SHADER];
	}
}

void GMGLGraphicEngine::newFrameOnCurrentFramebuffer()
{
	GLint mask;
	glGetIntegerv(GL_STENCIL_WRITEMASK, &mask);
	glStencilMask(0xFF);
	GM_BEGIN_CHECK_GL_ERROR
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	GM_END_CHECK_GL_ERROR
	glStencilMask(mask);
}

void GMGLGraphicEngine::clearStencilOnCurrentFramebuffer()
{
	GLint mask;
	glGetIntegerv(GL_STENCIL_WRITEMASK, &mask);
	glStencilMask(0xFF);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilMask(mask);
}

//////////////////////////////////////////////////////////////////////////
void GMGLUtility::blendFunc(GMS_BlendFunc sfactor, GMS_BlendFunc dfactor)
{
	GM_BEGIN_CHECK_GL_ERROR
	GMS_BlendFunc gms_factors[] = {
		sfactor,
		dfactor
	};
	GLenum factors[2];

	for (GMint i = 0; i < GM_array_size(gms_factors); i++)
	{
		switch (gms_factors[i])
		{
		case GMS_BlendFunc::ZERO:
			factors[i] = GL_ZERO;
			break;
		case GMS_BlendFunc::ONE:
			factors[i] = GL_ONE;
			break;
		case GMS_BlendFunc::SRC_COLOR:
			factors[i] = GL_SRC_COLOR;
			break;
		case GMS_BlendFunc::DST_COLOR:
			factors[i] = GL_DST_COLOR;
			break;
		case GMS_BlendFunc::SRC_ALPHA:
			factors[i] = GL_SRC_ALPHA;
			break;
		case GMS_BlendFunc::DST_ALPHA:
			factors[i] = GL_DST_ALPHA;
			break;
		case GMS_BlendFunc::ONE_MINUS_SRC_ALPHA:
			factors[i] = GL_ONE_MINUS_SRC_ALPHA;
			break;
		case GMS_BlendFunc::ONE_MINUS_DST_COLOR:
			factors[i] = GL_ONE_MINUS_DST_COLOR;
			break;
		case GMS_BlendFunc::ONE_MINUS_DST_ALPHA:
			factors[i] = GL_ONE_MINUS_DST_ALPHA;
			break;
		default:
			GM_ASSERT(false);
			break;
		}
	}
	glBlendFunc(factors[0], factors[1]);
	GM_END_CHECK_GL_ERROR
}