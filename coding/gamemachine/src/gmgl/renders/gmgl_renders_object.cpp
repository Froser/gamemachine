#include "stdafx.h"
#include "gmgl_renders_object.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmgl/shader_constants.h"
#include "gmgl/gmgltexture.h"
#include "gmengine/gmgameworld.h"
#include "foundation/linearmath.h"
#include "foundation/gamemachine.h"

GMGLRenders_Object::GMGLRenders_Object()
{
	D(d);
	d->engine = static_cast<GMGLGraphicEngine*>(GameMachine::instance().getGraphicEngine());
}

void GMGLRenders_Object::activateShader()
{
	D(d);
	ASSERT(d->shader);
	Shader* shader = d->shader;
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

		glEnable(GL_CULL_FACE);
	}

	if (shader->getBlend())
	{
		glEnable(GL_BLEND);
		GLenum factors[2];
		GMS_BlendFunc gms_factors[] = {
			shader->getBlendFactorSource(),
			shader->getBlendFactorDest(),
		};

		for (GMuint i = 0; i < 2; i++)
		{
			switch (gms_factors[i])
			{
			case GMS_BlendFunc::ZERO:
				factors[i] = GL_ZERO;
				break;
			case GMS_BlendFunc::ONE:
				factors[i] = GL_ONE;
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
			default:
				ASSERT(false);
				break;
			}
		}
		glBlendFunc(factors[0], factors[1]);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	if (shader->getBlend())
		glDepthMask(GL_FALSE);

	if (shader->getNoDepthTest())
		glDisable(GL_DEPTH_TEST); //glDepthMask(GL_FALSE);
	else
		glEnable(GL_DEPTH_TEST); // glDepthMask(GL_TRUE);

	glLineWidth(shader->getLineWidth());
}

void GMGLRenders_Object::begin(IGraphicEngine* engine, GMMesh* mesh, GMfloat* modelTransform)
{
	D(d);
	d->mesh = mesh;
	d->type = mesh->getType();
	d->shader = nullptr;

	auto shaderProgram = getShaderProgram();
	if (modelTransform)
		shaderProgram->setMatrix4(GMSHADER_MODEL_MATRIX, modelTransform);
}

void GMGLRenders_Object::beginShader(Shader& shader, GMDrawMode mode)
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
	GM_FOREACH_ENUM_CLASS(type, GMTextureType, GMTextureType::AMBIENT, GMTextureType::END)
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

void GMGLRenders_Object::endShader()
{
	D(d);
	if (d->shader->getBlend())
		glDepthMask(GL_TRUE);

	GM_FOREACH_ENUM_CLASS(type, GMTextureType, GMTextureType::AMBIENT, GMTextureType::END)
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

void GMGLRenders_Object::drawTexture(GMTextureType type, GMint index)
{
	D(d);
	if (GMGetBuiltIn(DRAW_LIGHTMAP_ONLY) && type != GMTextureType::LIGHTMAP)
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

void GMGLRenders_Object::end()
{
}

void GMGLRenders_Object::activateLight(const GMLight& light, GMint lightIndex)
{
	D(d);
	auto shaderProgram = getShaderProgram();

	switch (light.getType())
	{
	case GMLightType::AMBIENT:
		{
			const char* uniform = getLightUniformName(GMLightType::AMBIENT, lightIndex);
			char u_color[GMGL_MAX_UNIFORM_NAME_LEN];
			combineUniform(u_color, uniform, GMSHADER_LIGHTS_LIGHTCOLOR);
			shaderProgram->setVec3(u_color, light.getLightColor());
		}
		break;
	case GMLightType::SPECULAR:
		{
			const char* uniform = getLightUniformName(GMLightType::SPECULAR, lightIndex);
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

void GMGLRenders_Object::updateVPMatrices(const linear_math::Matrix4x4& projection, const linear_math::Matrix4x4& view, const CameraLookAt& lookAt)
{
	D(d);
	auto shaderProgram = getShaderProgram();

	// P
	shaderProgram->setMatrix4(GMSHADER_PROJECTION_MATRIX, projection.data());

	// V
	GMfloat vec[4] = { lookAt.position[0], lookAt.position[1], lookAt.position[2], 1.0f };
	shaderProgram->setMatrix4(GMSHADER_VIEW_POSITION, vec);

	// 视觉位置，用于计算光照
	shaderProgram->setMatrix4(GMSHADER_VIEW_MATRIX, view.data());
}

GMGLShaderProgram* GMGLRenders_Object::getShaderProgram()
{
	D(d);
	if (!d->gmglShaderProgram || d->renderMode != d->engine->getRenderMode())
		d->gmglShaderProgram = d->engine->getShaders(d->type);
	d->gmglShaderProgram->useProgram();
	d->renderMode = d->engine->getRenderMode();
	return d->gmglShaderProgram;
}

ITexture* GMGLRenders_Object::getTexture(GMTextureFrames& frames)
{
	D(d);
	if (frames.getFrameCount() == 0)
		return nullptr;

	if (frames.getFrameCount() == 1)
		return frames.getOneFrame(0);

	// 如果frameCount > 1，说明是个动画，要根据Shader的间隔来选择合适的帧
	// TODO
	GMint elapsed = GameMachine::instance().getGameTimeSeconds() * 1000;

	return frames.getOneFrame((elapsed / frames.getAnimationMs()) % frames.getFrameCount());
}

void GMGLRenders_Object::activateMaterial(const Shader& shader)
{
	D(d);
	const GMMaterial& material = shader.getMaterial();
	auto shaderProgram = getShaderProgram();
	shaderProgram->setVec3(GMSHADER_MATERIAL_KA, &material.ka[0]);
	shaderProgram->setVec3(GMSHADER_MATERIAL_KD, &material.kd[0]);
	shaderProgram->setVec3(GMSHADER_MATERIAL_KS, &material.ks[0]);
	shaderProgram->setFloat(GMSHADER_MATERIAL_SHININESS, material.shininess);
}

void GMGLRenders_Object::drawDebug()
{
	D(d);
	auto shaderProgram = getShaderProgram();
	shaderProgram->setInt(GMSHADER_DEBUG_DRAW_NORMAL, GMGetBuiltIn(DRAW_NORMAL));
}

void GMGLRenders_Object::activateTextureTransform(GMTextureType type, GMint index)
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

void GMGLRenders_Object::activateTexture(GMTextureType type, GMint index)
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

void GMGLRenders_Object::deactivateTexture(GMTextureType type, GMint index)
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

void GMGLRenders_Object::getTextureID(GMTextureType type, GMint index, REF GLenum& tex, REF GMint& texId)
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
		ASSERT(false);
		return;
	}
}