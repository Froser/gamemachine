﻿#include "stdafx.h"
#include <GL/glew.h>
#include "gmglrenderers.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmgl/shader_constants.h"
#include "gmgl/gmgltexture.h"
#include "gmengine/gmgameworld.h"
#include <linearmath.h>
#include "foundation/gamemachine.h"

namespace
{
	inline void applyShader(const GMShader& shader)
	{
		if (shader.getBlend())
		{
			glEnable(GL_BLEND);
			GMGLUtility::blendFunc(shader.getBlendFactorSource(), shader.getBlendFactorDest());

			GM_BEGIN_CHECK_GL_ERROR
			glDisable(GL_CULL_FACE);
			GM_END_CHECK_GL_ERROR
		}
		else
		{
			GM_BEGIN_CHECK_GL_ERROR
			glDisable(GL_BLEND);

			if (shader.getCull() == GMS_Cull::NONE)
			{
				glDisable(GL_CULL_FACE);
			}
			else
			{
				if (shader.getFrontFace() == GMS_FrontFace::CLOCKWISE)
					glFrontFace(GL_CW);
				else
					glFrontFace(GL_CCW);

				glEnable(GL_CULL_FACE);
			}
			GM_END_CHECK_GL_ERROR
		}

		GM_BEGIN_CHECK_GL_ERROR
		if (shader.getNoDepthTest())
			glDisable(GL_DEPTH_TEST); // glDepthMask(GL_FALSE);
		else
			glEnable(GL_DEPTH_TEST); // glDepthMask(GL_TRUE);
		GM_END_CHECK_GL_ERROR

		GM_BEGIN_CHECK_GL_ERROR
		if (shader.getBlend())
			glDepthMask(GL_FALSE);
		GM_END_CHECK_GL_ERROR

		glLineWidth(shader.getLineWidth());
	}

	inline void applyStencil(GMGLGraphicEngine& engine)
	{
		// 应用模板
		GLenum compareOp = GL_ALWAYS;
		auto& stencilOptions = engine.getStencilOptions();
		if (stencilOptions.compareOp == GMStencilOptions::Equal)
			compareOp = GL_EQUAL;
		else if (stencilOptions.compareOp == GMStencilOptions::NotEqual)
			compareOp = GL_NOTEQUAL;
		glStencilFunc(compareOp, 1, 0xFF);
		glStencilMask(stencilOptions.writeMask);
	}

	inline GLenum getMode(GMMesh* obj)
	{
		switch (obj->getArrangementMode())
		{
		case GMArrangementMode::Triangle_Fan:
			return GL_TRIANGLE_FAN;
		case GMArrangementMode::Triangle_Strip:
			return GL_TRIANGLE_STRIP;
		case GMArrangementMode::Triangles:
			return GL_TRIANGLES;
		case GMArrangementMode::Lines:
			return GL_LINE_LOOP;
		default:
			GM_ASSERT(false);
			return GL_TRIANGLE_FAN;
		}
	}
}

GMGLRenderer::GMGLRenderer()
{
	D(d);
	d->engine = gm_static_cast<GMGLGraphicEngine*>(GM.getGraphicEngine());
}

void GMGLRenderer::draw(IQueriable* painter, GMComponent* component, GMMesh* mesh)
{
	D(d);
	applyStencil(*d->engine);
	beforeDraw(component);
	GLenum mode = GMGetDebugState(POLYGON_LINE_MODE) ? GL_LINE_LOOP : getMode(mesh);
	glMultiDrawArrays(mode, (GLint*)component->getOffsetPtr(), (GLsizei*) component->getPrimitiveVerticesCountPtr(), component->getPrimitiveCount());
	afterDraw();
}

void GMGLRenderer_3D::beginModel(GMModel* model, const GMGameObject* parent)
{
	D(d);
	auto shaderProgram = GM.getGraphicEngine()->getShaderProgram();
	shaderProgram->useProgram();
	auto& desc = shaderProgram->getDesc();

	GM_BEGIN_CHECK_GL_ERROR
	shaderProgram->setInt(GMSHADER_SHADER_TYPE, (GMint)model->getType());
	GM_END_CHECK_GL_ERROR

	if (parent)
	{
		GM_BEGIN_CHECK_GL_ERROR
		shaderProgram->setMatrix4(desc.ModelMatrix, parent->getTransform());
		shaderProgram->setMatrix4(desc.InverseTransposeModelMatrix, InverseTranspose(parent->getTransform()));
		GM_END_CHECK_GL_ERROR
	}
	else
	{
		GM_BEGIN_CHECK_GL_ERROR
		shaderProgram->setMatrix4(desc.ModelMatrix, Identity<GMMat4>());
		shaderProgram->setMatrix4(desc.InverseTransposeModelMatrix, Identity<GMMat4>());
		GM_END_CHECK_GL_ERROR
	}
}

void GMGLRenderer_3D::beforeDraw(GMComponent* component)
{
	D(d);
	d->shader = &component->getShader();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// 材质
	activateMaterial(*d->shader);

	// 应用Shader
	applyShader(*d->shader);

	// 纹理
	GM_FOREACH_ENUM_CLASS(type, GMTextureType::AMBIENT, GMTextureType::END)
	{
		GMint count = GMMaxTextureCount(type);
		for (GMint i = 0; i < count; i++)
		{
			drawTexture((GMTextureType)type, i);
		}
	}

	// 调试绘制
	drawDebug();
}

void GMGLRenderer_3D::afterDraw()
{
	D(d);
	if (d->shader->getBlend())
		glDepthMask(GL_TRUE);

	GM_FOREACH_ENUM_CLASS(type, GMTextureType::AMBIENT, GMTextureType::END)
	{
		GMint count = GMMaxTextureCount(type);
		for (GMint i = 0; i < count; i++)
		{
			deactivateTexture((GMTextureType)type, i);
		}
	}
}

void GMGLRenderer_3D::drawTexture(GMTextureType type, GMint index)
{
	D(d);
	if (GMGetDebugState(DRAW_LIGHTMAP_ONLY) && type != GMTextureType::LIGHTMAP)
		return;

	// 按照贴图类型选择纹理动画序列
	GMTextureFrames& textures = d->shader->getTexture().getTextureFrames(type, index);

	// 获取序列中的这一帧
	ITexture* texture = getTexture(textures);
	if (texture)
	{
		// 激活动画序列
		activateTexture((GMTextureType)type, index);
		texture->drawTexture(&textures, index);
	}
}

void GMGLRenderer_3D::endModel()
{
}

ITexture* GMGLRenderer_3D::getTexture(GMTextureFrames& frames)
{
	D(d);
	if (frames.getFrameCount() == 0)
		return nullptr;

	if (frames.getFrameCount() == 1)
		return frames.getFrameByIndex(0);

	// 如果frameCount > 1，说明是个动画，要根据Shader的间隔来选择合适的帧
	// TODO
	GMint elapsed = GM.getGameTimeSeconds() * 1000;

	return frames.getFrameByIndex((elapsed / frames.getAnimationMs()) % frames.getFrameCount());
}

void GMGLRenderer_3D::activateMaterial(const GMShader& shader)
{
	D(d);
	auto shaderProgram = GM.getGraphicEngine()->getShaderProgram();
	auto& svd = shaderProgram->getDesc();
	static const std::string GMSHADER_MATERIAL_KA = std::string(svd.MaterialName) + "." + svd.MaterialAttributes.Ka;
	static const std::string GMSHADER_MATERIAL_KD = std::string(svd.MaterialName) + "." + svd.MaterialAttributes.Kd;
	static const std::string GMSHADER_MATERIAL_KS = std::string(svd.MaterialName) + "." + svd.MaterialAttributes.Ks;
	static const std::string GMSHADER_MATERIAL_SHININESS = std::string(svd.MaterialName) + "." + svd.MaterialAttributes.Shininess;
	static const std::string GMSHADER_MATERIAL_REFRACTIVITY = std::string(svd.MaterialName) + "." + svd.MaterialAttributes.Refreactivity;

	const GMMaterial& material = shader.getMaterial();
	shaderProgram->setVec3(GMSHADER_MATERIAL_KA.c_str(), ValuePointer(material.ka));
	shaderProgram->setVec3(GMSHADER_MATERIAL_KD.c_str(), ValuePointer(material.kd));
	shaderProgram->setVec3(GMSHADER_MATERIAL_KS.c_str(), ValuePointer(material.ks));
	shaderProgram->setFloat(GMSHADER_MATERIAL_SHININESS.c_str(), material.shininess);
	shaderProgram->setFloat(GMSHADER_MATERIAL_REFRACTIVITY.c_str(), material.refractivity);
}

void GMGLRenderer_3D::drawDebug()
{
	D(d);
	auto shaderProgram = GM.getGraphicEngine()->getShaderProgram();
	shaderProgram->setInt(GMSHADER_DEBUG_DRAW_NORMAL, GMGetDebugState(DRAW_NORMAL));
}

void GMGLRenderer_3D::activateTextureTransform(GMTextureType type, GMint index)
{
	D(d);
	const GMShaderVariablesDesc* desc = getVariablesDesc();
	GM_ASSERT(desc);
	static const std::string u_scrolls_affix = (GMString(".") + desc->TextureAttributes.OffsetX).toStdString();
	static const std::string u_scrollt_affix = (GMString(".") + desc->TextureAttributes.OffsetY).toStdString();
	static const std::string u_scales_affix = (GMString(".") + desc->TextureAttributes.ScaleX).toStdString();
	static const std::string u_scalet_affix = (GMString(".") + desc->TextureAttributes.ScaleY).toStdString();

	auto shaderProgram = GM.getGraphicEngine()->getShaderProgram();
	const char* uniform = getTextureUniformName(desc, type, index);

	char u_scrolls[GMGL_MAX_UNIFORM_NAME_LEN],
		u_scrollt[GMGL_MAX_UNIFORM_NAME_LEN],
		u_scales[GMGL_MAX_UNIFORM_NAME_LEN],
		u_scalet[GMGL_MAX_UNIFORM_NAME_LEN];

	combineUniform(u_scrolls, uniform, u_scrolls_affix.c_str());
	combineUniform(u_scrollt, uniform, u_scrollt_affix.c_str());
	combineUniform(u_scales, uniform, u_scales_affix.c_str());
	combineUniform(u_scalet, uniform, u_scalet_affix.c_str());
	shaderProgram->setFloat(u_scrolls, 0.f);
	shaderProgram->setFloat(u_scrollt, 0.f);
	shaderProgram->setFloat(u_scales, 1.f);
	shaderProgram->setFloat(u_scalet, 1.f);

	auto applyCallback = [&](GMS_TextureModType type, Pair<GMfloat, GMfloat>&& args) {
		if (type == GMS_TextureModType::SCALE)
		{
			shaderProgram->setFloat(u_scales, args.first);
			shaderProgram->setFloat(u_scalet, args.second);
		}
		else if (type == GMS_TextureModType::SCROLL)
		{
			shaderProgram->setFloat(u_scrolls, args.first);
			shaderProgram->setFloat(u_scrollt, args.second);
		}
		else
		{
			GM_ASSERT(false);
		}
	};

	d->shader->getTexture().getTextureFrames(type, index).applyTexMode(GM.getGameTimeSeconds(), applyCallback);
}

void GMGLRenderer_3D::activateTexture(GMTextureType type, GMint index)
{
	D(d);
	const GMShaderVariablesDesc* desc = getVariablesDesc();
	GM_ASSERT(desc);
	static const std::string u_tex_enabled = (GMString(".") + desc->TextureAttributes.Enabled).toStdString();
	static const std::string u_tex_texture = (GMString(".") + desc->TextureAttributes.Texture).toStdString();

	GMint idx = (GMint)type;
	GLenum tex;
	GMint texId;
	getTextureID(type, index, tex, texId);

	auto shaderProgram = GM.getGraphicEngine()->getShaderProgram();
	const char* uniform = getTextureUniformName(desc, type, index);
	char u_texture[GMGL_MAX_UNIFORM_NAME_LEN], u_enabled[GMGL_MAX_UNIFORM_NAME_LEN];
	combineUniform(u_texture, uniform, u_tex_texture.c_str());
	combineUniform(u_enabled, uniform, u_tex_enabled.c_str());
	shaderProgram->setInt(u_texture, texId);
	shaderProgram->setInt(u_enabled, 1);

	activateTextureTransform(type, index);
	glActiveTexture(tex);
}

void GMGLRenderer_3D::deactivateTexture(GMTextureType type, GMint index)
{
	D(d);
	const GMShaderVariablesDesc* desc = getVariablesDesc();
	GM_ASSERT(desc);
	static const std::string u_tex_enabled = (GMString(".") + desc->TextureAttributes.Enabled).toStdString();

	GLenum tex;
	GMint texId;
	getTextureID(type, index, tex, texId);

	auto shaderProgram = GM.getGraphicEngine()->getShaderProgram();
	GMint idx = (GMint)type;
	const char* uniform = getTextureUniformName(desc, type, index);
	char u[GMGL_MAX_UNIFORM_NAME_LEN];
	combineUniform(u, uniform, u_tex_enabled.c_str());
	shaderProgram->setInt(u, 0);
}

void GMGLRenderer_3D::getTextureID(GMTextureType type, GMint index, REF GLenum& tex, REF GMint& texId)
{
	switch (type)
	{
	case GMTextureType::AMBIENT:
	case GMTextureType::DIFFUSE:
		texId = (GMint)type * GMMaxTextureCount(type) + index + 1;
		tex = texId - 1 + GL_TEXTURE1;
		break;
	case GMTextureType::NORMALMAP:
		texId = 7;
		tex = GL_TEXTURE7;
		break;
	case GMTextureType::LIGHTMAP:
		texId = 8;
		tex = GL_TEXTURE8;
		break;
	default:
		GM_ASSERT(false);
		return;
	}
}

//////////////////////////////////////////////////////////////////////////
void GMGLRenderer_2D::beforeDraw(GMComponent* component)
{
	D(d);
	d->shader = &component->getShader();

	// 应用Shader
	applyShader(*d->shader);

	// 只选择环境光纹理
	GMTextureFrames& textures = d->shader->getTexture().getTextureFrames(GMTextureType::AMBIENT, 0);

	// 获取序列中的这一帧
	ITexture* texture = getTexture(textures);
	if (texture)
	{
		// 激活动画序列
		activateTexture(GMTextureType::AMBIENT, 0);
		texture->drawTexture(&textures, 0);
	}
}

//////////////////////////////////////////////////////////////////////////
void GMGLRenderer_CubeMap::beginModel(GMModel* model, const GMGameObject* parent)
{
	D(d);
	d->cubemap = GMObject::gmobject_cast<const GMCubeMapGameObject*>(parent);

	IShaderProgram* shaderProgram = GM.getGraphicEngine()->getShaderProgram();
	shaderProgram->useProgram();
	auto& desc = shaderProgram->getDesc();

	GM_BEGIN_CHECK_GL_ERROR
	shaderProgram->setInt(GMSHADER_SHADER_TYPE, (GMint)model->getType());
	shaderProgram->setMatrix4(desc.ModelMatrix, GMMat4(Inhomogeneous(parent->getTransform())));
	shaderProgram->setMatrix4(desc.InverseTransposeModelMatrix, GMMat4(Inhomogeneous(parent->getTransform())));
	GM_END_CHECK_GL_ERROR
}

void GMGLRenderer_CubeMap::endModel()
{
	D(d);
	d->cubemap = nullptr;
}

void GMGLRenderer_CubeMap::beforeDraw(GMComponent* component)
{
	D(d);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	GMTexture& texture = component->getShader().getTexture();
	GMTextureFrames& frames = texture.getTextureFrames(GMTextureType::CUBEMAP, 0);
	ITexture* glTex = frames.getFrameByIndex(0);
	if (glTex)
	{
		GM_BEGIN_CHECK_GL_ERROR
		IGraphicEngine* engine = GM.getGraphicEngine();
		// 给延迟渲染程序传入CubeMap
		{
			IShaderProgram* shader = engine->getShaderProgram(GMShaderProgramType::DeferredGeometryPassShaderProgram);
			shader->useProgram();
			shader->setInt(GMSHADER_CUBEMAP_TEXTURE, CubeMapActiveTexture);
		}
		{
			IShaderProgram* shader = engine->getShaderProgram(GMShaderProgramType::DeferredLightPassShaderProgram);
			shader->useProgram();
			shader->setInt(GMSHADER_CUBEMAP_TEXTURE, CubeMapActiveTexture);
		}

		// 给正向渲染程序传入CubeMap，一定要放在最后，因为CubeMap渲染本身是正向渲染
		{
			IShaderProgram* shader = engine->getShaderProgram(GMShaderProgramType::ForwardShaderProgram);
			shader->useProgram();
			shader->setInt(GMSHADER_CUBEMAP_TEXTURE, CubeMapActiveTexture);
		}
		GM_END_CHECK_GL_ERROR

		GM_BEGIN_CHECK_GL_ERROR
		glActiveTexture(GL_TEXTURE0 + CubeMapActiveTexture);
		GM_END_CHECK_GL_ERROR

		GM_BEGIN_CHECK_GL_ERROR
		glTex->drawTexture(&frames, 0);
		GM_END_CHECK_GL_ERROR
	}
}

void GMGLRenderer_CubeMap::afterDraw()
{
}