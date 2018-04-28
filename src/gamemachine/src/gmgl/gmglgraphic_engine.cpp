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
#include "gmglframebuffer.h"

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

GMGLGraphicEngine::~GMGLGraphicEngine()
{
	D(d);
	GM_delete(d->filterShaderProgram);
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

#if _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback((GLDEBUGPROC)GL_MessageCallback, 0);
#endif
}

void GMGLGraphicEngine::newFrame()
{
	D(d);
	GLint mask;
	glGetIntegerv(GL_STENCIL_WRITEMASK, &mask);
	glStencilMask(0xFF);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(mask);
}

void GMGLGraphicEngine::installShaders()
{
	D(d);
	if (!getShaderLoadCallback())
	{
		gm_error("You must specify a IShaderLoadCallback");
		GM_ASSERT(false);
		return;
	}

	getShaderLoadCallback()->onLoadShaders(this);
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

void GMGLGraphicEngine::activateLights(IShaderProgram* shaderProgram)
{
	D(d);
	D_BASE(db, Base);
	// 有2种情况，需要更新着色器。
	// 1. 使用中的着色器更换时
	// 2. 使用中的着色器未更换，但是光照信息改变

	if (shaderProgram != d->lightContext.shaderProgram || d->lightContext.lightDirty)
	{
		/*
		auto& svd = shaderProgram->getDesc();
		shaderProgram->useProgram();
		GMint lightId[(GMuint)GMLightType::COUNT] = { 0 };
		for (auto& light : db->lights)
		{
			GMint id = lightId[(GMuint)light.getType()]++;
			switch (light.getType())
			{
			case GMLightType::AMBIENT:
			{
				const char* uniform = getLightUniformName(svd, GMLightType::AMBIENT, id);
				char u_color[GMGL_MAX_UNIFORM_NAME_LEN];
				combineUniform(u_color, uniform, ".", svd.LightCount);
				shaderProgram->setVec3(u_color, light.getLightColor());
				break;
			}
			case GMLightType::SPECULAR:
			{
				const char* uniform = getLightUniformName(svd, GMLightType::SPECULAR, id);
				char u_color[GMGL_MAX_UNIFORM_NAME_LEN], u_position[GMGL_MAX_UNIFORM_NAME_LEN];
				combineUniform(u_color, uniform, ".", svd.LightCount);
				combineUniform(u_position, uniform, ".", svd.Light);
				shaderProgram->setVec3(u_color, light.getLightColor());
				shaderProgram->setVec3(u_position, light.getLightPosition());
				break;
			}
			default:
				break;
			}
		}
		shaderProgram->setInt(svd.AmbientLight.Count, lightId[(GMint)GMLightType::AMBIENT]);
		shaderProgram->setInt(svd.SpecularLight.Count, lightId[(GMint)GMLightType::SPECULAR]);
		d->lightContext.shaderProgram = shaderProgram;
		d->lightContext.lightDirty = false;
		*/
	}
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
	static GMGLRenderer_2D s_renderer2d;
	static GMGLRenderer_3D s_renderer3d;
	static GMGLRenderer_CubeMap s_rendererCubeMap;
	static GMGLRenderer_Filter s_rendererFilter;
	static GMGLRenderer_LightPass s_rendererLightPass;
	switch (objectType)
	{
	case GMModelType::Model2D:
	case GMModelType::Glyph:
		return &s_renderer2d;
	case GMModelType::Model3D:
		return &s_renderer3d;
	case GMModelType::CubeMap:
		return &s_rendererCubeMap;
	case GMModelType::Filter:
		return &s_rendererFilter;
	case GMModelType::LightPassQuad:
		return &s_rendererLightPass;
	default:
		GM_ASSERT(false);
		return nullptr;
	}
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
	return GMGLFramebuffers::getDefaultFramebuffers();
}

//////////////////////////////////////////////////////////////////////////
void GMGLUtility::blendFunc(GMS_BlendFunc sfactor, GMS_BlendFunc dfactor)
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
}