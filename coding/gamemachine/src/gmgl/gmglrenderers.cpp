#include "stdafx.h"
#include "gmglrenderers.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmgl/shader_constants.h"
#include "gmgl/gmgltexture.h"
#include "gmengine/gmgameworld.h"
#include <linearmath.h>
#include "foundation/gamemachine.h"

namespace
{
	void applyShader(const GMShader& shader)
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
}

GMGLRenderer_3D::GMGLRenderer_3D()
{
	D(d);
	d->engine = static_cast<GMGLGraphicEngine*>(GameMachine::instance().getGraphicEngine());
}

void GMGLRenderer_3D::beginModel(GMModel* model, const GMGameObject* parent)
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
	else
	{
		GM_BEGIN_CHECK_GL_ERROR
		shaderProgram->setMatrix4(GMSHADER_MODEL_MATRIX, glm::value_ptr(glm::identity<glm::mat4>()));
		GM_END_CHECK_GL_ERROR
	}
}

void GMGLRenderer_3D::beginComponent(GMComponent* component)
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

void GMGLRenderer_3D::endComponent()
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
		texture->drawTexture(&textures);
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
	const GMMaterial& material = shader.getMaterial();
	auto shaderProgram = d->engine->getShaderProgram();
	shaderProgram->setVec3(GMSHADER_MATERIAL_KA, &material.ka[0]);
	shaderProgram->setVec3(GMSHADER_MATERIAL_KD, &material.kd[0]);
	shaderProgram->setVec3(GMSHADER_MATERIAL_KS, &material.ks[0]);
	shaderProgram->setFloat(GMSHADER_MATERIAL_SHININESS, material.shininess);
}

void GMGLRenderer_3D::drawDebug()
{
	D(d);
	auto shaderProgram = d->engine->getShaderProgram();
	shaderProgram->setInt(GMSHADER_DEBUG_DRAW_NORMAL, GMGetDebugState(DRAW_NORMAL));
}

void GMGLRenderer_3D::activateTextureTransform(GMTextureType type, GMint index)
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

void GMGLRenderer_3D::activateTexture(GMTextureType type, GMint index)
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

void GMGLRenderer_3D::deactivateTexture(GMTextureType type, GMint index)
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
void GMGLRenderer_2D::beginComponent(GMComponent* component)
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
		texture->drawTexture(&textures);
	}
}

//////////////////////////////////////////////////////////////////////////
void GMGLRenderer_CubeMap::beginModel(GMModel* model, const GMGameObject* parent)
{
	IShaderProgram* shaderProgram = ((GMGLGraphicEngine*)GM.getGraphicEngine())->getShaderProgram();
	shaderProgram->useProgram();

	GM_BEGIN_CHECK_GL_ERROR
	shaderProgram->setInt(GMSHADER_SHADER_TYPE, (GMint)model->getType());
	shaderProgram->setMatrix4(GMSHADER_MODEL_MATRIX, glm::value_ptr(glm::mat4(glm::mat3(parent->getTransform()))));
	GM_END_CHECK_GL_ERROR
}

void GMGLRenderer_CubeMap::endModel()
{

}

void GMGLRenderer_CubeMap::beginComponent(GMComponent* component)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	applyShader(component->getShader());

	GMTexture& texture = component->getShader().getTexture();
	GMTextureFrames& frames = texture.getTextureFrames(GMTextureType::CUBEMAP, 0);
	ITexture* glTex = frames.getFrameByIndex(0);
	if (glTex)
	{
		GM_BEGIN_CHECK_GL_ERROR
		GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(GM.getGraphicEngine());
		auto shaderProgram = engine->getShaderProgram();
		shaderProgram->setInt(GMSHADER_CUBEMAP_TEXTURE, 0);
		GM_END_CHECK_GL_ERROR

		GM_BEGIN_CHECK_GL_ERROR
		glActiveTexture(GL_TEXTURE0);
		GM_END_CHECK_GL_ERROR

		GM_BEGIN_CHECK_GL_ERROR
		glTex->drawTexture(&frames);
		GM_END_CHECK_GL_ERROR
	}
}

void GMGLRenderer_CubeMap::endComponent()
{
}