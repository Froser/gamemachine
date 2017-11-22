#include "stdafx.h"
#include "gmgl_renders_3d.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmgl/shader_constants.h"
#include "gmgl/gmgltexture.h"
#include "gmengine/gmgameworld.h"
#include <linearmath.h>
#include "foundation/gamemachine.h"

GMGLRenders_3D::GMGLRenders_3D()
{
	D(d);
	d->engine = static_cast<GMGLGraphicEngine*>(GameMachine::instance().getGraphicEngine());
}

void GMGLRenders_3D::activateShader()
{
	D(d);
	GM_ASSERT(d->shader);
	Shader* shader = d->shader;
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

void GMGLRenders_3D::begin(IGraphicEngine* engine, GMMesh* mesh, GMfloat* modelTransform)
{
	D(d);
	d->mesh = mesh;
	d->type = mesh->getType();
	d->shader = nullptr;

	auto shaderProgram = getShaderProgram();
	if (modelTransform)
		shaderProgram->setMatrix4(GMSHADER_MODEL_MATRIX, modelTransform);
}

void GMGLRenders_3D::beginShader(Shader& shader, GMDrawMode mode)
{
	D(d);
	d->shader = &shader;
	d->mode = mode;

	if (mode == GMDrawMode::Fill)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else
	{
		d->shader->stash();

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(0.f, 1.f);

		// 设置边框颜色
		shader.getMaterial().ka = d->shader->getLineColor();
	}

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

void GMGLRenders_3D::endShader()
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

	if (d->mode == GMDrawMode::Line)
		d->shader->pop();
}

void GMGLRenders_3D::drawTexture(GMTextureType type, GMint index)
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

void GMGLRenders_3D::end()
{
}

void GMGLRenders_3D::activateLights(const GMLight* lights, GMint count)
{
	D(d);
	if (!count)
		return;

	auto shaderProgram = getShaderProgram();

	GMint lightId[(GMuint)GMLightType::COUNT] = { 0 };
	for (GMint i = 0; i < count; i++)
	{
		const GMLight& light = lights[i];
		GMint id = lightId[(GMuint)light.getType()]++;

		switch (light.getType())
		{
		case GMLightType::AMBIENT:
			{
				const char* uniform = getLightUniformName(GMLightType::AMBIENT, id);
				char u_color[GMGL_MAX_UNIFORM_NAME_LEN];
				combineUniform(u_color, uniform, GMSHADER_LIGHTS_LIGHTCOLOR);
				shaderProgram->setVec3(u_color, light.getLightColor());
			}
			break;
		case GMLightType::SPECULAR:
			{
				const char* uniform = getLightUniformName(GMLightType::SPECULAR, id);
				char u_color[GMGL_MAX_UNIFORM_NAME_LEN], u_position[GMGL_MAX_UNIFORM_NAME_LEN];
				combineUniform(u_color, uniform, GMSHADER_LIGHTS_LIGHTCOLOR);
				combineUniform(u_position, uniform, GMSHADER_LIGHTS_LIGHTPOSITION);
				shaderProgram->setVec3(u_color, light.getLightColor());
				shaderProgram->setVec3(u_position, light.getLightPosition());
			}
			break;
		default:
			break;
		}
	}

	shaderProgram->setInt(GMSHADER_AMBIENTS_COUNT, lightId[(GMint)GMLightType::AMBIENT]);
	shaderProgram->setInt(GMSHADER_SPECULARS_COUNT, lightId[(GMint)GMLightType::SPECULAR]);
}

void GMGLRenders_3D::updateVPMatrices(const linear_math::Matrix4x4& projection, const linear_math::Matrix4x4& view, const CameraLookAt& lookAt)
{
	D(d);
	auto shaderProgram = getShaderProgram();

	// P
	shaderProgram->setMatrix4(GMSHADER_PROJECTION_MATRIX, projection.data());

	// 视觉位置，用于计算光照
	GMfloat vec[4] = { lookAt.position[0], lookAt.position[1], lookAt.position[2], 1.0f };
	shaderProgram->setMatrix4(GMSHADER_VIEW_POSITION, vec);

	// V
	shaderProgram->setMatrix4(GMSHADER_VIEW_MATRIX, view.data());
}

GMGLShaderProgram* GMGLRenders_3D::getShaderProgram()
{
	D(d);
	if (!d->gmglShaderProgram ||
		d->renderMode != GMGetRenderState(RENDER_MODE) ||
		d->renderState != d->engine->getRenderState())
	{
		d->gmglShaderProgram = d->engine->getShaders(d->type);
	}
	d->gmglShaderProgram->useProgram();
	d->renderMode = GMGetRenderState(RENDER_MODE);
	d->renderState = d->engine->getRenderState();
	return d->gmglShaderProgram;
}

ITexture* GMGLRenders_3D::getTexture(GMTextureFrames& frames)
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

void GMGLRenders_3D::activateMaterial(const Shader& shader)
{
	D(d);
	const GMMaterial& material = shader.getMaterial();
	auto shaderProgram = getShaderProgram();
	shaderProgram->setVec3(GMSHADER_MATERIAL_KA, &material.ka[0]);
	shaderProgram->setVec3(GMSHADER_MATERIAL_KD, &material.kd[0]);
	shaderProgram->setVec3(GMSHADER_MATERIAL_KS, &material.ks[0]);
	shaderProgram->setFloat(GMSHADER_MATERIAL_SHININESS, material.shininess);
}

void GMGLRenders_3D::drawDebug()
{
	D(d);
	auto shaderProgram = getShaderProgram();
	shaderProgram->setInt(GMSHADER_DEBUG_DRAW_NORMAL, GMGetDebugState(DRAW_NORMAL));
}

void GMGLRenders_3D::activateTextureTransform(GMTextureType type, GMint index)
{
	D(d);
	auto shaderProgram = getShaderProgram();
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

void GMGLRenders_3D::activateTexture(GMTextureType type, GMint index)
{
	D(d);
	GMint idx = (GMint)type;

	GLenum tex;
	GMint texId;
	getTextureID(type, index, tex, texId);

	auto shaderProgram = getShaderProgram();
	const char* uniform = getTextureUniformName(type, index);
	char u_texture[GMGL_MAX_UNIFORM_NAME_LEN], u_enabled[GMGL_MAX_UNIFORM_NAME_LEN];
	combineUniform(u_texture, uniform, GMSHADER_TEXTURES_TEXTURE);
	combineUniform(u_enabled, uniform, GMSHADER_TEXTURES_ENABLED);
	shaderProgram->setInt(u_texture, texId);
	shaderProgram->setInt(u_enabled, 1);

	activateTextureTransform(type, index);
	glActiveTexture(tex);
}

void GMGLRenders_3D::deactivateTexture(GMTextureType type, GMint index)
{
	D(d);
	GLenum tex;
	GMint texId;
	getTextureID(type, index, tex, texId);

	auto shaderProgram = getShaderProgram();
	GMint idx = (GMint)type;
	const char* uniform = getTextureUniformName(type, index);
	char u[GMGL_MAX_UNIFORM_NAME_LEN];
	combineUniform(u, uniform, GMSHADER_TEXTURES_ENABLED);
	shaderProgram->setInt(u, 0);
}

void GMGLRenders_3D::getTextureID(GMTextureType type, GMint index, REF GLenum& tex, REF GMint& texId)
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