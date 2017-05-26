#include "stdafx.h"
#include "gmgl_renders_object.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmgl/gmglfunc.h"
#include "gmgl/shader_constants.h"
#include "gmgl/gmgltexture.h"
#include "gmengine/elements/gameworld.h"
#include "foundation/linearmath.h"

void GMGLRenders_Object::activateShader(Shader* shader)
{
	if (shader->cull == GMS_NONE)
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glFrontFace(GL_CW);
		glEnable(GL_CULL_FACE);
	}

	if (shader->blend)
	{
		glEnable(GL_BLEND);
		GLenum factors[2];
		for (GMuint i = 0; i < 2; i++)
		{
			switch (shader->blendFactors[i])
			{
			case GMS_ZERO:
				factors[i] = GL_ZERO;
				break;
			case GMS_ONE:
				factors[i] = GL_ONE;
				break;
			case GMS_DST_COLOR:
				factors[i] = GL_DST_COLOR;
				break;
			case GMS_SRC_ALPHA:
				factors[i] = GL_SRC_ALPHA;
				break;
			case GMS_ONE_MINUS_SRC_ALPHA:
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

	if (shader->blend)
		glDepthMask(GL_FALSE);

	if (shader->noDepthTest)
		glDisable(GL_DEPTH_TEST); //glDepthMask(GL_FALSE);
	else
		glEnable(GL_DEPTH_TEST); // glDepthMask(GL_TRUE);
}

void GMGLRenders_Object::deactivateShader(Shader* shader)
{
	if (shader->blend)
	{
		glDepthMask(GL_TRUE);
	}
}

void GMGLRenders_Object::begin(IGraphicEngine* engine, Mesh* mesh, GMfloat* modelTransform)
{
	D(d);
	clearData();
	d->engine = static_cast<GMGLGraphicEngine*>(engine);
	d->mesh = mesh;
	d->type = mesh->getType();
	d->gmglShaders = d->engine->getShaders(d->type);
	d->world = d->engine->getWorld();
	d->gmglShaders->useProgram();

	if (modelTransform)
		GMGL::uniformMatrix4(*d->gmglShaders, modelTransform, GMSHADER_MODEL_MATRIX);
}

void GMGLRenders_Object::beginShader(Shader& shader)
{
	D(d);
	d->shader = &shader;

	// 光照
	for (GMint i = LT_BEGIN; i < LT_END; i++)
	{
		activateLight((LightType)i, shader.lights[i]);
	}

	// 纹理
	activateShader(&shader);

	for (GMuint i = 0; i < TEXTURE_INDEX_MAX; i++)
	{
		// 按照贴图类型选择纹理动画序列
		TextureFrames& textures = shader.texture.textures[i];

		// 获取序列中的这一帧
		ITexture* texture = getTexture(textures);
		if (texture)
		{
			// 激活动画序列
			activeTexture(d->shader, (TextureIndex)i);
			texture->drawTexture(&textures);
		}
	}

	// 调试绘制
	drawDebug();
}

void GMGLRenders_Object::endShader()
{
	D(d);
	deactivateShader(d->shader);
	for (GMuint i = 0; i < TEXTURE_INDEX_MAX; i++)
	{
		deactiveTexture((TextureIndex)i);
	}
}

void GMGLRenders_Object::end()
{
}

void GMGLRenders_Object::updateVPMatrices(const linear_math::Matrix4x4& projection, const linear_math::Matrix4x4& view, const CameraLookAt& lookAt)
{
	D(d);
	GMGL::perspective(projection, *d->gmglShaders, GMSHADER_PROJECTION_MATRIX);
	GMGL::lookAt(view, *d->gmglShaders, GMSHADER_VIEW_MATRIX);
	GMGL::cameraPosition(lookAt, *d->gmglShaders, GMSHADER_VIEW_POSITION);
}

void GMGLRenders_Object::clearData()
{
	D(d);
	memset(&d, 0, sizeof(d));
}

ITexture* GMGLRenders_Object::getTexture(TextureFrames& frames)
{
	D(d);
	if (frames.frameCount == 0)
		return nullptr;

	if (frames.frameCount == 1)
		return frames.frames[0];

	// 如果frameCount > 1，说明是个动画，要根据Shader的间隔来选择合适的帧
	// TODO
	GMint elapsed = d->world->getElapsed() * 1000;

	return frames.frames[(elapsed / frames.animationMs) % frames.frameCount];
}

void GMGLRenders_Object::activateLight(LightType t, LightInfo& light)
{
	D(d);
	switch (t)
	{
	case gm::LT_AMBIENT:
	{
		GMGL::uniformVec3(*d->gmglShaders, light.on && !light.useGlobalLightColor ? &light.lightColor[0] : &(d->world->getDefaultAmbientLight().lightColor)[0], GMSHADER_LIGHT_AMBIENT);
		GMGL::uniformVec3(*d->gmglShaders, light.on ? &light.args[LA_KA] : &d->world->getDefaultAmbientLight().args[LA_KA], GMSHADER_LIGHT_KA);
	}
	break;
	case gm::LT_SPECULAR:
	{
		GMfloat zero[3] = { 0 };
		GMGL::uniformVec3(*d->gmglShaders, light.on ? (!light.useGlobalLightColor ? &light.lightColor[0] : &(d->world->getDefaultAmbientLight().lightColor)[0]) : zero, GMSHADER_LIGHT_POWER);
		GMGL::uniformVec3(*d->gmglShaders, light.on ? &light.args[LA_KD] : zero, GMSHADER_LIGHT_KD);
		GMGL::uniformVec3(*d->gmglShaders, light.on ? &light.args[LA_KS] : zero, GMSHADER_LIGHT_KS);
		GMGL::uniformFloat(*d->gmglShaders, light.on ? light.args[LA_SHINESS] : 0.f, GMSHADER_LIGHT_SHININESS);
	}
	break;
	default:
		ASSERT(false);
		break;
	}
}

void GMGLRenders_Object::drawDebug()
{
	D(d);
	GMGL::uniformInt(*d->gmglShaders, DBG_INT(DRAW_NORMAL), GMSHADER_DEBUG_DRAW_NORMAL);
}

void GMGLRenders_Object::activeTextureTransform(Shader* shader, TextureIndex i)
{
	D(d);
	std::string uniform = getTextureUniformName(i);

	const std::string SCROLL_S = std::string(uniform).append("_scroll_s");
	const std::string SCROLL_T = std::string(uniform).append("_scroll_t");
	const std::string SCALE_S = std::string(uniform).append("_scale_s");
	const std::string SCALE_T = std::string(uniform).append("_scale_t");

	glUniform1f(glGetUniformLocation(d->gmglShaders->getProgram(), SCROLL_S.c_str()), 0.f);
	glUniform1f(glGetUniformLocation(d->gmglShaders->getProgram(), SCROLL_T.c_str()), 0.f);
	glUniform1f(glGetUniformLocation(d->gmglShaders->getProgram(), SCALE_S.c_str()), 1.f);
	glUniform1f(glGetUniformLocation(d->gmglShaders->getProgram(), SCALE_T.c_str()), 1.f);

	GMuint n = 0;
	while (n < MAX_TEX_MOD && shader->texture.textures[i].texMod[n].type != GMS_NO_TEXTURE_MOD)
	{
		GMS_TextureMod* tc = &shader->texture.textures[i].texMod[n];
		switch (tc->type)
		{
		case GMS_SCROLL:
		{
			GMfloat s = d->world->getElapsed() * tc->p1, t = d->world->getElapsed() * tc->p2;
			glUniform1f(glGetUniformLocation(d->gmglShaders->getProgram(), SCROLL_T.c_str()), t);
			glUniform1f(glGetUniformLocation(d->gmglShaders->getProgram(), SCROLL_T.c_str()), s);
		}
		break;
		case GMS_SCALE:
		{
			GMfloat s = tc->p1, t = tc->p2;
			glUniform1f(glGetUniformLocation(d->gmglShaders->getProgram(), SCALE_T.c_str()), t);
			glUniform1f(glGetUniformLocation(d->gmglShaders->getProgram(), SCALE_T.c_str()), s);
			break;
		}
		default:
			break;
		}
		n++;
	}
}

void GMGLRenders_Object::activeTexture(Shader* shader, TextureIndex i)
{
	D(d);
	std::string uniform = getTextureUniformName(i);
	GLint loc = glGetUniformLocation(d->gmglShaders->getProgram(), uniform.c_str());
	glUniform1i(loc, i + 1);
	loc = glGetUniformLocation(d->gmglShaders->getProgram(), std::string(uniform).append("_switch").c_str());
	glUniform1i(loc, 1);

	activeTextureTransform(shader, i);
	glActiveTexture(i + GL_TEXTURE1);
}

void GMGLRenders_Object::deactiveTexture(TextureIndex i)
{
	D(d);
	std::string uniform = getTextureUniformName(i);
	GMint loc = glGetUniformLocation(d->gmglShaders->getProgram(), std::string(uniform).append("_switch").c_str());
	glUniform1i(loc, 0);
	glActiveTexture(i + GL_TEXTURE1);
}