#include "stdafx.h"
#include "gmgl_renders_object.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmgl/shader_constants.h"
#include "gmgl/gmgltexture.h"
#include "gmengine/gmgameworld.h"
#include "foundation/linearmath.h"
#include "foundation/gamemachine.h"

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

void GMGLRenders_Object::deactivateShader()
{
	D(d);
	ASSERT(d->shader);
	if (d->shader->getBlend())
	{
		glDepthMask(GL_TRUE);
	}
}

void GMGLRenders_Object::begin(IGraphicEngine* engine, GMMesh* mesh, GMfloat* modelTransform)
{
	D(d);
	clearData();
	d->engine = static_cast<GMGLGraphicEngine*>(GameMachine::instance().getGraphicEngine());
	d->mesh = mesh;
	d->type = mesh->getType();
	d->gmglShaderProgram = d->engine->getShaders(d->type);
	d->gmglShaderProgram->useProgram();

	if (modelTransform)
		d->gmglShaderProgram->setMatrix4(GMSHADER_MODEL_MATRIX, modelTransform);
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
		GMLight& light = d->shader->getLight(LT_AMBIENT);
		light.setLightColor(d->shader->getLineColor());
		light.setKa(linear_math::Vector3(1, 1, 1));
		light.setEnabled(true);
		light.setUseGlobalLightColor(false);
	}

	// 光照
	for (GMint i = LT_BEGIN; i < LT_END; i++)
	{
		activateLight((LightType)i, shader.getLight((LightType)i));
	}

	// 应用Shader
	activateShader();

	// 纹理
	for (GMuint type = 0; type < (GMint)GMTextureType::END; type++)
	{
		if (type != (GMint) GMTextureType::NORMALMAP)
		{
			for (GMint i = 0; i < MAX_TEXTURE_COUNT; i++)
			{
				drawTexture((GMTextureType)type, i);
			}
		}
		else
		{
			drawTexture((GMTextureType)type);
		}
	}

	// 调试绘制
	drawDebug();
}

void GMGLRenders_Object::endShader()
{
	D(d);
	deactivateShader();
	for (GMuint type = 0; type < (GMint)GMTextureType::END; type++)
	{
		if (type != (GMint)GMTextureType::NORMALMAP)
		{
			for (GMint i = 0; i < MAX_TEXTURE_COUNT; i++)
			{
				deactivateTexture((GMTextureType)type, i);
			}
		}
		else
		{
			drawTexture((GMTextureType)type);
		}
	}

	if (d->mode == GMDrawMode::Line)
	{
		// 还原所有线条Shader
		glDisable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(0.f, 0.f);
		d->shader->pop();
	}
}

void GMGLRenders_Object::drawTexture(GMTextureType type, GMint index)
{
	D(d);
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

void GMGLRenders_Object::updateVPMatrices(const linear_math::Matrix4x4& projection, const linear_math::Matrix4x4& view, const CameraLookAt& lookAt)
{
	D(d);
	// P
	d->gmglShaderProgram->setMatrix4(GMSHADER_PROJECTION_MATRIX, projection.data());

	// V
	GMfloat vec[4] = { lookAt.position[0], lookAt.position[1], lookAt.position[2], 1.0f };
	d->gmglShaderProgram->setMatrix4(GMSHADER_VIEW_POSITION, vec);

	// 视觉位置，用于计算光照
	d->gmglShaderProgram->setMatrix4(GMSHADER_VIEW_MATRIX, view.data());
}

void GMGLRenders_Object::clearData()
{
	D(d);
	memset(&d, 0, sizeof(d));
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

void GMGLRenders_Object::activateLight(LightType t, GMLight& light)
{
	D(d);
	switch (t)
	{
	case gm::LT_AMBIENT:
	{
		GMfloat* defaultLight = d->engine->getEnvironment().ambientLightColor;
		GMfloat* defaultKa = d->engine->getEnvironment().ambientK;
		d->gmglShaderProgram->setVec3(GMSHADER_LIGHT_AMBIENT, light.getEnabled() && !light.getUseGlobalLightColor() ? &light.getLightColor()[0] : defaultLight);
		d->gmglShaderProgram->setVec3(GMSHADER_LIGHT_KA, light.getEnabled() ? &light.getKa()[0] : defaultKa);
	}
	break;
	case gm::LT_SPECULAR:
	{
		GMfloat* defaultLight = d->engine->getEnvironment().ambientLightColor;
		GMfloat zero[3] = { 0 };
		d->gmglShaderProgram->setVec3(GMSHADER_LIGHT_POWER, light.getEnabled() ? (!light.getUseGlobalLightColor() ? &light.getLightColor()[0] : defaultLight) : zero);
		d->gmglShaderProgram->setVec3(GMSHADER_LIGHT_KD, light.getEnabled() ? &light.getKd()[0] : zero);
		d->gmglShaderProgram->setVec3(GMSHADER_LIGHT_KS, light.getEnabled() ? &light.getKs()[0] : zero);
		d->gmglShaderProgram->setFloat(GMSHADER_LIGHT_SHININESS, light.getEnabled() ? light.getShininess() : 0.f);
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
	d->gmglShaderProgram->setInt(GMSHADER_DEBUG_DRAW_NORMAL, GMGetBuiltIn(DRAW_NORMAL));
}

void GMGLRenders_Object::activateTextureTransform(GMTextureType i, GMint index)
{
	D(d);
	auto uniform = getTextureUniformName(i, index);

	const GMString SCROLL_S = uniform + ".scroll_s";
	const GMString SCROLL_T = uniform + ".scroll_t";
	const GMString SCALE_S = uniform + ".scale_s";
	const GMString SCALE_T = uniform + ".scale_t";

	d->gmglShaderProgram->setFloat(SCROLL_S, 0.f);
	d->gmglShaderProgram->setFloat(SCROLL_T, 0.f);
	d->gmglShaderProgram->setFloat(SCALE_S, 1.f);
	d->gmglShaderProgram->setFloat(SCALE_T, 1.f);

	GMuint n = 0;
	const GMS_TextureMod* tc = &d->shader->getTexture().getTextureFrames(i).getTexMod(n);
	while (n < MAX_TEX_MOD && tc->type != GMS_TextureModType::NO_TEXTURE_MOD)
	{
		switch (tc->type)
		{
		case GMS_TextureModType::SCROLL:
		{
			GMfloat s = GameMachine::instance().getGameTimeSeconds() * tc->p1, t = GameMachine::instance().getGameTimeSeconds() * tc->p2;
			d->gmglShaderProgram->setFloat(SCROLL_T, t);
			d->gmglShaderProgram->setFloat(SCROLL_T, s);
		}
		break;
		case GMS_TextureModType::SCALE:
		{
			GMfloat s = tc->p1, t = tc->p2;
			d->gmglShaderProgram->setFloat(SCALE_T, t);
			d->gmglShaderProgram->setFloat(SCALE_T, s);
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
	auto uniform = getTextureUniformName(type, index);
	d->gmglShaderProgram->setInt(uniform + ".texture", idx + 1);
	d->gmglShaderProgram->setInt(uniform + ".enabled", 1);

	activateTextureTransform(type, index);
	glActiveTexture(idx + GL_TEXTURE1);
}

void GMGLRenders_Object::deactivateTexture(GMTextureType type, GMint index)
{
	D(d);
	GMint idx = (GMint)type;
	auto uniform = getTextureUniformName(type, index);
	d->gmglShaderProgram->setInt(uniform + "enabled", 0);
	glActiveTexture(idx + GL_TEXTURE1);
}