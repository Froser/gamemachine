#include "stdafx.h"
#include "gmgl_render_3d.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmgl/shader_constants.h"
#include "gmgl/gmgltexture.h"
#include "gmengine/gmgameworld.h"
#include <linearmath.h>
#include "foundation/gamemachine.h"

GMGLRender_3D::GMGLRender_3D()
{
	D(d);
	d->engine = static_cast<GMGLGraphicEngine*>(GameMachine::instance().getGraphicEngine());
}

void GMGLRender_3D::activateShader()
{
	D(d);
	GM_ASSERT(d->shader);
	GMShader* shader = d->shader;
	if (shader->getBlend())
	{
		glEnable(GL_BLEND);
		GMGLUtility::blendFunc(shader->getBlendFactorSource(), shader->getBlendFactorDest());

		glDisable(GL_CULL_FACE);
		GM_CHECK_GL_ERROR();
	}
	else
	{
		glDisable(GL_BLEND);
		GM_CHECK_GL_ERROR();

		if (shader->getCull() == GMS_Cull::NONE)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			if (shader->getFrontFace() == GMS_FrontFace::CLOCKWISE)
				glFrontFace(GL_CW);
			else
				glFrontFace(GL_CCW);
			GM_CHECK_GL_ERROR();

			glEnable(GL_CULL_FACE);
		}
	}

	if (shader->getNoDepthTest())
		glDisable(GL_DEPTH_TEST); //glDepthMask(GL_FALSE);
	else
		glEnable(GL_DEPTH_TEST); // glDepthMask(GL_TRUE);

	if (shader->getBlend())
		glDepthMask(GL_FALSE);

	GM_CHECK_GL_ERROR();
	glLineWidth(shader->getLineWidth());
}

void GMGLRender_3D::begin(GMModel* model, const GMGameObject* parent)
{
	D(d);
	auto shaderProgram = d->engine->getShaderProgram();
	shaderProgram->useProgram();

	shaderProgram->setInt(GMSHADER_SHADER_TYPE, (GMint)model->getType());
	if (parent)
	{
		GM_BEGIN_CHECK_GL_ERROR
		shaderProgram->setMatrix4(GMSHADER_MODEL_MATRIX, glm::value_ptr(parent->getTransform()));
		GM_END_CHECK_GL_ERROR
	}
}

void GMGLRender_3D::beginShader(GMShader& shader)
{
	D(d);
	d->shader = &shader;
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// 材质
	activateMaterial(shader);

	// 应用Shader
	activateShader();

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

void GMGLRender_3D::endShader()
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

void GMGLRender_3D::drawTexture(GMTextureType type, GMint index)
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
		texture->drawTexture(&textures);
	}
}

void GMGLRender_3D::end()
{
}

ITexture* GMGLRender_3D::getTexture(GMTextureFrames& frames)
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

void GMGLRender_3D::activateMaterial(const GMShader& shader)
{
	D(d);
	const GMMaterial& material = shader.getMaterial();
	auto shaderProgram = d->engine->getShaderProgram();
	shaderProgram->setVec3(GMSHADER_MATERIAL_KA, &material.ka[0]);
	shaderProgram->setVec3(GMSHADER_MATERIAL_KD, &material.kd[0]);
	shaderProgram->setVec3(GMSHADER_MATERIAL_KS, &material.ks[0]);
	shaderProgram->setFloat(GMSHADER_MATERIAL_SHININESS, material.shininess);
}

void GMGLRender_3D::drawDebug()
{
	D(d);
	auto shaderProgram = d->engine->getShaderProgram();
	shaderProgram->setInt(GMSHADER_DEBUG_DRAW_NORMAL, GMGetDebugState(DRAW_NORMAL));
}

void GMGLRender_3D::activateTextureTransform(GMTextureType type, GMint index)
{
	D(d);
	auto shaderProgram = d->engine->getShaderProgram();
	const char* uniform = getTextureUniformName(type, index);
	char u_scrolls[GMGL_MAX_UNIFORM_NAME_LEN],
		u_scrollt[GMGL_MAX_UNIFORM_NAME_LEN],
		u_scales[GMGL_MAX_UNIFORM_NAME_LEN],
		u_scalet[GMGL_MAX_UNIFORM_NAME_LEN];

	combineUniform(u_scrolls, uniform, GMSHADER_TEXTURES_SCROLL_S);
	combineUniform(u_scrollt, uniform, GMSHADER_TEXTURES_SCROLL_T);
	combineUniform(u_scales, uniform, GMSHADER_TEXTURES_SCALE_S);
	combineUniform(u_scalet, uniform, GMSHADER_TEXTURES_SCALE_T);
	shaderProgram->setFloat(u_scrolls, 0.f);
	shaderProgram->setFloat(u_scrollt, 0.f);
	shaderProgram->setFloat(u_scales, 1.f);
	shaderProgram->setFloat(u_scalet, 1.f);

	GMuint n = 0;
	const GMS_TextureMod* tc = &d->shader->getTexture().getTextureFrames(type, index).getTexMod(n);
	while (n < MAX_TEX_MOD && tc->type != GMS_TextureModType::NO_TEXTURE_MOD)
	{
		switch (tc->type)
		{
		case GMS_TextureModType::SCROLL:
		{
			GMfloat s = GameMachine::instance().getGameTimeSeconds() * tc->p1, t = GameMachine::instance().getGameTimeSeconds() * tc->p2;
			shaderProgram->setFloat(u_scrollt, t);
			shaderProgram->setFloat(u_scrollt, s);
		}
		break;
		case GMS_TextureModType::SCALE:
		{
			GMfloat s = tc->p1, t = tc->p2;
			shaderProgram->setFloat(u_scales, t);
			shaderProgram->setFloat(u_scales, s);
			break;
		}
		default:
			break;
		}
		n++;
	}
}

void GMGLRender_3D::activateTexture(GMTextureType type, GMint index)
{
	D(d);
	GMint idx = (GMint)type;

	GLenum tex;
	GMint texId;
	getTextureID(type, index, tex, texId);

	auto shaderProgram = d->engine->getShaderProgram();
	const char* uniform = getTextureUniformName(type, index);
	char u_texture[GMGL_MAX_UNIFORM_NAME_LEN], u_enabled[GMGL_MAX_UNIFORM_NAME_LEN];
	combineUniform(u_texture, uniform, GMSHADER_TEXTURES_TEXTURE);
	combineUniform(u_enabled, uniform, GMSHADER_TEXTURES_ENABLED);
	shaderProgram->setInt(u_texture, texId);
	shaderProgram->setInt(u_enabled, 1);

	activateTextureTransform(type, index);
	glActiveTexture(tex);
}

void GMGLRender_3D::deactivateTexture(GMTextureType type, GMint index)
{
	D(d);
	GLenum tex;
	GMint texId;
	getTextureID(type, index, tex, texId);

	auto shaderProgram = d->engine->getShaderProgram();
	GMint idx = (GMint)type;
	const char* uniform = getTextureUniformName(type, index);
	char u[GMGL_MAX_UNIFORM_NAME_LEN];
	combineUniform(u, uniform, GMSHADER_TEXTURES_ENABLED);
	shaderProgram->setInt(u, 0);
}

void GMGLRender_3D::getTextureID(GMTextureType type, GMint index, REF GLenum& tex, REF GMint& texId)
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