#include "stdafx.h"
#include <GL/glew.h>
#include "shader_constants.h"
#include <algorithm>
#include "gmglgraphic_engine.h"
#include "gmdata/gmmodel.h"
#include "gmgltexture.h"
#include "gmglmodeldataproxy.h"
#include "gmgltechniques.h"
#include "foundation/gamemachine.h"
#include "foundation/gmconfigs.h"
#include "foundation/gmprofile.h"
#include "gmglframebuffer.h"
#include "gmglglyphmanager.h"

extern "C"
{
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
				gm_error(gm_dbg_wrap("GL CALLBACK: {0} type = {1}, severity = {2}, message = {3}"),
					(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
					GMString(static_cast<GMint32>(type)), GMString(static_cast<GMint32>(severity)), message);
			}
			else if (severity == GL_DEBUG_SEVERITY_LOW)
			{
				gm_warning(gm_dbg_wrap("GL CALLBACK: {0} type = {1}, severity = {2}, message = {3}"),
				(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
					GMString(static_cast<GMint32>(type)), GMString(static_cast<GMint32>(severity)), message);
			}
		}
	}
}

GMGLGraphicEngine::GMGLGraphicEngine(const IRenderContext* context)
	: GMGraphicEngine(context)
{
}

void GMGLGraphicEngine::init()
{
	Base::init();
	installShaders();
	glEnable(GL_MULTISAMPLE);

	auto& runningState = GM.getRunningStates();
	glClearDepth(runningState.farZ);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_STENCIL_TEST);
	glClearStencil(0);
	GMStencilOptions options(GMStencilOptions::Ox00, GMStencilOptions::Always);
	setStencilOptions(options);
	getDefaultFramebuffers()->clear();

#if GM_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback((GLDEBUGPROC)GL_MessageCallback, 0);
#endif
}

void GMGLGraphicEngine::installShaders()
{
	D_BASE(d, Base);
	if (!getShaderLoadCallback())
	{
		gm_error(gm_dbg_wrap("You must specify a IShaderLoadCallback"));
		GM_ASSERT(false);
		return;
	}

	getShaderLoadCallback()->onLoadShaders(d->context);
	d->renderTechniqueManager->init();
}

bool GMGLGraphicEngine::setInterface(GameMachineInterfaceID id, void* in)
{
	D(d);
	switch (id)
	{
	case GameMachineInterfaceID::GLFiltersShaderProgram:
		d->filterShaderProgram.reset(static_cast<GMGLShaderProgram*>(in));
		break;
	case GameMachineInterfaceID::GLForwardShaderProgram:
		d->forwardShaderProgram.reset(static_cast<GMGLShaderProgram*>(in));
		break;
	case GameMachineInterfaceID::GLDeferredShaderGeometryProgram:
		d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER].reset(static_cast<GMGLShaderProgram*>(in));
		break;
	case GameMachineInterfaceID::GLDeferredShaderLightProgram:
		d->deferredShaderPrograms[DEFERRED_LIGHT_PASS_SHADER].reset(static_cast<GMGLShaderProgram*>(in));
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

void GMGLGraphicEngine::activateLights(ITechnique* technique)
{
	D(d);
	D_BASE(db, Base);
	// 有2种情况，需要更新着色器。
	// 1. 使用中的着色器更换时
	// 2. 使用中的着色器未更换，但是光照信息改变
	GMGLTechnique* glTechnique = gm_cast<GMGLTechnique*>(technique);
	IShaderProgram* shaderProgram = glTechnique->getShaderProgram();
	if (shaderProgram != d->lightContext.shaderProgram || d->lightContext.lightDirty)
	{
		const Vector<ILight*>& lights = db->lights;
		GMsize_t lightCount = lights.size();
		GM_ASSERT(lightCount <= getMaxLightCount());
		shaderProgram->setInt(getVariableIndex(shaderProgram, d->lightCountIndices[verifyIndicesContainer(d->lightCountIndices, shaderProgram)], GM_VariablesDesc.LightCount) , gm_sizet_to_uint(lightCount));

		for (GMuint32 i = 0; i < gm_sizet_to_uint(lightCount); ++i)
		{
			lights[i]->activateLight(i, technique);
		}
		d->lightContext.shaderProgram = shaderProgram;
		d->lightContext.lightDirty = false;
	}
}

void GMGLGraphicEngine::shaderProgramChanged(IShaderProgram* program)
{
	GMGLTechnique::dirtyShadowMapAttributes();
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
		d->cubeMap = GMAsset::invalidAsset();
		break;
	}
	default:
		GM_ASSERT(false);
		break;
	}
}

ITechnique* GMGLGraphicEngine::getTechnique(GMModelType objectType)
{
	D(d);
	D_BASE(db, Base);

	switch (objectType)
	{
	case GMModelType::Model2D:
	case GMModelType::Text:
		return newTechnique<GMGLTechnique_2D>(d->technique_2d, db->context);
	case GMModelType::Model3D:
		if (db->isDrawingShadow)
			return newTechnique<GMGLTechnique_3D_Shadow>(d->technique_3d_shadow, db->context);
		return newTechnique<GMGLTechnique_3D>(d->technique_3d, db->context);
	case GMModelType::CubeMap:
		return newTechnique<GMGLTechnique_CubeMap>(d->technique_cubeMap, db->context);
	case GMModelType::Filter:
		return newTechnique<GMGLTechnique_Filter>(d->technique_filter, db->context);
	case GMModelType::LightPassQuad:
		return newTechnique<GMGLTechnique_LightPass>(d->technique_lightPass, db->context);
	case GMModelType::Particle:
		return newTechnique<GMGLTechnique_Particle>(d->technique_particle, db->context);
	case GMModelType::Custom:
		return newTechnique<GMGLTechnique_Custom>(d->technique_custom, db->context);
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

IShaderProgram* GMGLGraphicEngine::getShaderProgram(GMShaderProgramType type)
{
	D(d);
	if (type == GMShaderProgramType::DefaultShaderProgram)
	{
		return d->forwardShaderProgram.get();
	}
	else if (type == GMShaderProgramType::ForwardShaderProgram)
	{
		return d->forwardShaderProgram.get();
	}
	else if (type == GMShaderProgramType::DeferredGeometryPassShaderProgram)
	{
		return d->deferredShaderPrograms[DEFERRED_GEOMETRY_PASS_SHADER].get();
	}
	else if (type == GMShaderProgramType::FilterShaderProgram)
	{
		return d->filterShaderProgram.get();
	}
	else
	{
		GM_ASSERT(type == GMShaderProgramType::DeferredLightPassShaderProgram);
		return d->deferredShaderPrograms[DEFERRED_LIGHT_PASS_SHADER].get();
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
void GMGLUtility::blendFunc(
	GMS_BlendFunc sfactorRGB,
	GMS_BlendFunc dfactorRGB,
	GMS_BlendOp opRGB,
	GMS_BlendFunc sfactorAlpha,
	GMS_BlendFunc dfactorAlpha,
	GMS_BlendOp opAlpha
)
{
	GMS_BlendFunc gms_factors[] = {
		sfactorRGB,
		dfactorRGB,
		sfactorAlpha,
		dfactorAlpha
	};
	GMS_BlendOp gms_ops[] = {
		opRGB,
		opAlpha
	};

	GLenum factors[4];
	for (GMint32 i = 0; i < GM_array_size(gms_factors); i++)
	{
		switch (gms_factors[i])
		{
		case GMS_BlendFunc::Zero:
			factors[i] = GL_ZERO;
			break;
		case GMS_BlendFunc::One:
			factors[i] = GL_ONE;
			break;
		case GMS_BlendFunc::SourceColor:
			factors[i] = GL_SRC_COLOR;
			break;
		case GMS_BlendFunc::DestColor:
			factors[i] = GL_DST_COLOR;
			break;
		case GMS_BlendFunc::SourceAlpha:
			factors[i] = GL_SRC_ALPHA;
			break;
		case GMS_BlendFunc::DestAlpha:
			factors[i] = GL_DST_ALPHA;
			break;
		case GMS_BlendFunc::OneMinusSourceAlpha:
			factors[i] = GL_ONE_MINUS_SRC_ALPHA;
			break;
		case GMS_BlendFunc::OneMinusDestColor:
			factors[i] = GL_ONE_MINUS_DST_COLOR;
			break;
		case GMS_BlendFunc::OneMinusSourceColor:
			factors[i] = GL_ONE_MINUS_SRC_COLOR;
			break;
		case GMS_BlendFunc::OneMinusDestAlpha:
			factors[i] = GL_ONE_MINUS_DST_ALPHA;
			break;
		default:
			GM_ASSERT(false);
			break;
		}
	}
	glBlendFuncSeparate(factors[0], factors[1], factors[2], factors[3]);

	GLenum ops[2];
	for (GMint32 i = 0; i < GM_array_size(gms_ops); i++)
	{
		switch (gms_ops[i])
		{
		case GMS_BlendOp::Add:
			ops[i] = GL_FUNC_ADD;
			break;
		case GMS_BlendOp::Substract:
			ops[i] = GL_FUNC_SUBTRACT;
			break;
		case GMS_BlendOp::ReverseSubstract:
			ops[i] = GL_FUNC_REVERSE_SUBTRACT;
			break;
		default:
			ops[i] = GL_FUNC_ADD;
			gm_error(gm_dbg_wrap("Invalid blend op."));
		}
	}
	glBlendEquationSeparate(ops[0], ops[1]);
}

void GMGLGraphicEngine::clearGLErrors()
{
	while (glGetError() != GL_NO_ERROR)
	{
	}
}

void GMGLGraphicEngine::getGLErrors(GMuint32* errors, GMsize_t* count)
{
	GLenum errorCode = GL_NO_ERROR;
	GMsize_t cnt = 0;
	while ( (errorCode = glGetError()) != GL_NO_ERROR)
	{
		++cnt;
		if (errors)
			*errors++ = errorCode;

		if (cnt == GLMaxError)
		{
			clearGLErrors();
			break;
		}
	}
	
	if (errors)
		*errors++ = GL_NO_ERROR;
	
	if (count)
		*count = cnt;
}

void GMGLGraphicEngine::checkGLErrors(const GMuint32* errors)
{
	for (GMsize_t i = 0;; ++i)
	{
		if (errors[i] != GL_NO_ERROR)
			gm_error(gm_dbg_wrap("OpenGL error occurs: {0}"), GMString( (GMint32)errors[i] ));
		else
			break;
	}
}