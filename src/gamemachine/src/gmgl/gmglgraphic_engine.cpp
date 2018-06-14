﻿#include "stdafx.h"
#include <GL/glew.h>
#include "shader_constants.h"
#include <algorithm>
#include "gmglgraphic_engine.h"
#include "gmdata/gmmodel.h"
#include "gmgltexture.h"
#include "gmglmodeldataproxy.h"
#include "gmglrenderers.h"
#include "foundation/gamemachine.h"
#include "foundation/gmconfigs.h"
#include "foundation/gmprofile.h"
#include "gmglframebuffer.h"
#include "gmglglyphmanager.h"

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
			if (severity == GL_DEBUG_SEVERITY_MEDIUM || severity == GL_DEBUG_SEVERITY_HIGH)
			{
				gm_error("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
					(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
					type, severity, message);
			}
			else if (severity == GL_DEBUG_SEVERITY_LOW)
			{
				gm_warning("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
					(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
					type, severity, message);
			}
			else
			{
				gm_info("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
					(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
					type, severity, message);
			}
		}
	}
}

namespace
{
	template <typename T>
	IRenderer* newRenderer(IRenderer*& ptr, const IRenderContext* context)
	{
		if (!ptr)
			ptr = new T(context);
		return ptr;
	}
}

GMGLGraphicEngine::GMGLGraphicEngine(const IRenderContext* context)
	: GMGraphicEngine(context)
{
}

GMGLGraphicEngine::~GMGLGraphicEngine()
{
	D(d);
	GM_delete(d->filterShaderProgram);
	GM_delete(d->forwardShaderProgram);
	GM_delete(d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER]);
	GM_delete(d->deferredShaderPrograms[DEFERRED_LIGHT_PASS_SHADER]);

	GM_delete(d->renderer_2d);
	GM_delete(d->renderer_3d);
	GM_delete(d->renderer_cubeMap);
	GM_delete(d->renderer_filter);
	GM_delete(d->renderer_lightPass);
	GM_delete(d->renderer_3d_shadow);
}

void GMGLGraphicEngine::init()
{
	Base::init();
	installShaders();
	glEnable(GL_MULTISAMPLE);

	auto& runningState = GM.getGameMachineRunningStates();
	glClearDepth(runningState.farZ);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_STENCIL_TEST);
	glClearStencil(0);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

#if _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback((GLDEBUGPROC)GL_MessageCallback, 0);
#endif
}

void GMGLGraphicEngine::installShaders()
{
	D_BASE(d, Base);
	if (!getShaderLoadCallback())
	{
		gm_error("You must specify a IShaderLoadCallback");
		GM_ASSERT(false);
		return;
	}

	getShaderLoadCallback()->onLoadShaders(d->context);
}

bool GMGLGraphicEngine::setInterface(GameMachineInterfaceID id, void* in)
{
	D(d);
	switch (id)
	{
	case GameMachineInterfaceID::GLEffectShaderProgram:
		d->filterShaderProgram = static_cast<GMGLShaderProgram*>(in);
		d->filterShaderProgram->load();
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

void GMGLGraphicEngine::createShadowFramebuffers(OUT IFramebuffers** framebuffers)
{
	D_BASE(d, Base);
	GMGLShadowFramebuffers* sdframebuffers = new GMGLShadowFramebuffers(d->context);
	(*framebuffers) = sdframebuffers;

	GMFramebuffersDesc desc;
	GMRect rect;
	rect.width = d->shadow.width;
	rect.height = d->shadow.height;
	desc.rect = rect;
	bool succeed = sdframebuffers->init(desc);
	GM_ASSERT(succeed);
}

void GMGLGraphicEngine::activateLights(IRenderer* renderer)
{
	D(d);
	D_BASE(db, Base);
	// 有2种情况，需要更新着色器。
	// 1. 使用中的着色器更换时
	// 2. 使用中的着色器未更换，但是光照信息改变
	GMGLRenderer* glRenderer = gm_cast<GMGLRenderer*>(renderer);
	IShaderProgram* shaderProgram = glRenderer->getShaderProgram();
	if (shaderProgram != d->lightContext.shaderProgram || d->lightContext.lightDirty)
	{
		const Vector<ILight*>& lights = db->lights;
		GMsize_t lightCount = lights.size();
		GM_ASSERT(lightCount <= getMaxLightCount());
		GM_ASSERT(lightCount <= std::numeric_limits<GMuint>::max());
		shaderProgram->setInt(GM_VariablesDesc.LightCount, (GMuint)lightCount);

		for (GMuint i = 0; i < (GMuint)lightCount; ++i)
		{
			lights[i]->activateLight(i, renderer);
		}
		d->lightContext.shaderProgram = shaderProgram;
		d->lightContext.lightDirty = false;
	}
}

void GMGLGraphicEngine::shaderProgramChanged(IShaderProgram* program)
{
	GMGLRenderer::dirtyShadowMapAttributes();
}

void GMGLGraphicEngine::update(GMUpdateDataType type)
{
	D(d);
	switch (type)
	{
	case GMUpdateDataType::LightChanged:
		d->lightContext.lightDirty = true;
		break;
	case GMUpdateDataType::TurnOffCubeMap:
	{
		glActiveTexture(GL_TEXTURE0 + GMTextureRegisterQuery<GMTextureType::CubeMap>::Value);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		d->cubeMap = nullptr;
		break;
	}
	default:
		GM_ASSERT(false);
		break;
	}
}

IRenderer* GMGLGraphicEngine::getRenderer(GMModelType objectType)
{
	D(d);
	D_BASE(db, Base);

	switch (objectType)
	{
	case GMModelType::Model2D:
	case GMModelType::Text:
		return newRenderer<GMGLRenderer_2D>(d->renderer_2d, db->context);
	case GMModelType::Model3D:
		if (db->isDrawingShadow)
			return newRenderer<GMGLRenderer_3D_Shadow>(d->renderer_3d_shadow, db->context);
		return newRenderer<GMGLRenderer_3D>(d->renderer_3d, db->context);
	case GMModelType::CubeMap:
		return newRenderer<GMGLRenderer_CubeMap>(d->renderer_cubeMap, db->context);
	case GMModelType::Filter:
		return newRenderer<GMGLRenderer_Filter>(d->renderer_filter, db->context);
	case GMModelType::LightPassQuad:
		return newRenderer<GMGLRenderer_LightPass>(d->renderer_lightPass, db->context);
	default:
		GM_ASSERT(false);
		return nullptr;
	}
}

GMGlyphManager* GMGLGraphicEngine::getGlyphManager()
{
	D_BASE(d, Base);
	if (!d->glyphManager)
	{
		d->glyphManager = new GMGLGlyphManager(d->context);
	}
	return d->glyphManager;
}

void GMGLGraphicEngine::clearStencil()
{
	GLint mask;
	glGetIntegerv(GL_STENCIL_WRITEMASK, &mask);
	glStencilMask(0xFF);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilMask(mask);
}

void GMGLGraphicEngine::beginBlend(GMS_BlendFunc sfactor, GMS_BlendFunc dfactor)
{
	D(d);
	GM_ASSERT(!d->isBlending); // 不能重复进行多次Blend
	d->isBlending = true;
	d->blendsfactor = sfactor;
	d->blenddfactor = dfactor;
}

void GMGLGraphicEngine::endBlend()
{
	D(d);
	d->isBlending = false;
}

IShaderProgram* GMGLGraphicEngine::getShaderProgram(GMShaderProgramType type)
{
	D(d);
	if (type == GMShaderProgramType::DefaultShaderProgram)
	{
		return d->forwardShaderProgram;
	}
	else if (type == GMShaderProgramType::ForwardShaderProgram)
	{
		return d->forwardShaderProgram;
	}
	else if (type == GMShaderProgramType::DeferredGeometryPassShaderProgram)
	{
		return d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER];
	}
	else if (type == GMShaderProgramType::FilterShaderProgram)
	{
		return d->filterShaderProgram;
	}
	else
	{
		GM_ASSERT(type == GMShaderProgramType::DeferredLightPassShaderProgram);
		return d->deferredShaderPrograms[DEFERRED_LIGHT_PASS_SHADER];
	}
}

IFramebuffers* GMGLGraphicEngine::getDefaultFramebuffers()
{
	D_BASE(d, Base);
	if (!d->defaultFramebuffers)
		d->defaultFramebuffers = GMGLFramebuffers::createDefaultFramebuffers(d->context);
	return d->defaultFramebuffers;
}

//////////////////////////////////////////////////////////////////////////
void GMGLUtility::blendFunc(GMS_BlendFunc sfactor, GMS_BlendFunc dfactor, GMS_BlendOp op)
{
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

	switch (op)
	{
	case GMS_BlendOp::ADD:
		glBlendEquation(GL_FUNC_ADD);
		break;
	case GMS_BlendOp::SUBSTRACT:
		glBlendEquation(GL_FUNC_SUBTRACT);
		break;
	case GMS_BlendOp::REVERSE_SUBSTRACT:
		glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
		break;
	default:
		gm_error(L"Invalid blend op.");
	}
}