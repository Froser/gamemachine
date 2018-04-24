#include "stdafx.h"
#include <GL/glew.h>
#include "gmglrenderers.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmgl/shader_constants.h"
#include "gmgl/gmgltexture.h"
#include "gmengine/gmgameworld.h"
#include <linearmath.h>
#include "foundation/gamemachine.h"
#include "foundation/utilities/utilities.h"

namespace
{
	inline void applyShader(const GMShader& shader)
	{
		if (shader.getBlend())
		{
			glEnable(GL_BLEND);
			GMGLUtility::blendFunc(shader.getBlendFactorSource(), shader.getBlendFactorDest());
			
			glDisable(GL_CULL_FACE);
		}
		else
		{
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
			
		}

		if (shader.getNoDepthTest())
			glDisable(GL_DEPTH_TEST); // glDepthMask(GL_FALSE);
		else
			glEnable(GL_DEPTH_TEST); // glDepthMask(GL_TRUE);

		if (shader.getBlend())
			glDepthMask(GL_FALSE);

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

	inline GLenum getMode(GMTopologyMode mode)
	{
		switch (mode)
		{
		case GMTopologyMode::TriangleStrip:
			return GL_TRIANGLE_STRIP;
		case GMTopologyMode::Triangles:
			return GL_TRIANGLES;
		case GMTopologyMode::Lines:
			return GL_LINE_LOOP;
		default:
			GM_ASSERT(false);
			return GL_TRIANGLES;
		}
	}

	inline const char* getTechnique(GMModelType type)
	{
		switch (type)
		{
		case GMModelType::Model2D:
			return "GM_Model2D";
		case GMModelType::Model3D:
			return "GM_Model3D";
		case GMModelType::Glyph:
			return "GM_Glyph";
		case GMModelType::CubeMap:
			return "GM_CubeMap";
		default:
			GM_ASSERT(false);
			return "GM_Model2D";
		}
	}
}

GMGLRenderer::GMGLRenderer()
{
	D(d);
	d->engine = gm_static_cast<GMGLGraphicEngine*>(GM.getGraphicEngine());
	d->debugConfig = GM.getConfigs().getConfig(GMConfigs::Debug).asDebugConfig();
}

void GMGLRenderer::draw(GMModel* model)
{
	D(d);
	applyStencil(*d->engine);
	beforeDraw(model);
	GLenum mode = d->debugConfig.get(GMDebugConfigs::DrawPolygonsAsLine_Bool).toBool() ? GL_LINE_LOOP : getMode(model->getPrimitiveTopologyMode());
	if (model->getDrawMode() == GMModelDrawMode::Vertex)
		glDrawArrays(mode, 0, model->getVerticesCount());
	else
		glDrawElements(mode, model->getVerticesCount(), GL_UNSIGNED_INT, 0);
	afterDraw(model);
}

void GMGLRenderer::activateTextureTransform(GMModel* model, GMTextureType type, GMint index)
{
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

	model->getShader().getTexture().getTextureFrames(type, index).applyTexMode(GM.getGameTimeSeconds(), applyCallback);
}

GMint GMGLRenderer::activateTexture(GMModel* model, GMTextureType type, GMint index)
{
	const GMShaderVariablesDesc* desc = getVariablesDesc();
	GM_ASSERT(desc);
	static const std::string u_tex_enabled = (GMString(".") + desc->TextureAttributes.Enabled).toStdString();
	static const std::string u_tex_texture = (GMString(".") + desc->TextureAttributes.Texture).toStdString();

	GMint idx = (GMint)type;
	GMint texId = getTextureID(type, index);

	auto shaderProgram = GM.getGraphicEngine()->getShaderProgram();
	const char* uniform = getTextureUniformName(desc, type, index);
	char u_texture[GMGL_MAX_UNIFORM_NAME_LEN], u_enabled[GMGL_MAX_UNIFORM_NAME_LEN];
	combineUniform(u_texture, uniform, u_tex_texture.c_str());
	combineUniform(u_enabled, uniform, u_tex_enabled.c_str());
	shaderProgram->setInt(u_texture, texId);
	shaderProgram->setInt(u_enabled, 1);

	activateTextureTransform(model, type, index);
	return texId;
}

void GMGLRenderer::deactivateTexture(GMTextureType type, GMint index)
{
	const GMShaderVariablesDesc* desc = getVariablesDesc();
	GM_ASSERT(desc);
	static const std::string u_tex_enabled = (GMString(".") + desc->TextureAttributes.Enabled).toStdString();
	GMint texId = getTextureID(type, index);

	auto shaderProgram = GM.getGraphicEngine()->getShaderProgram();
	GMint idx = (GMint)type;
	const char* uniform = getTextureUniformName(desc, type, index);
	char u[GMGL_MAX_UNIFORM_NAME_LEN];
	combineUniform(u, uniform, u_tex_enabled.c_str());
	shaderProgram->setInt(u, 0);
}

GMint GMGLRenderer::getTextureID(GMTextureType type, GMint index)
{
	switch (type)
	{
	case GMTextureType::Ambient:
		return GMTextureRegisterQuery<GMTextureType::Ambient>::Value + index;
		break;
	case GMTextureType::Diffuse:
		return GMTextureRegisterQuery<GMTextureType::Diffuse>::Value + index;
		break;
	case GMTextureType::NormalMap:
		GM_ASSERT(index == 0);
		return GMTextureRegisterQuery<GMTextureType::NormalMap>::Value;
		break;
	case GMTextureType::Lightmap:
		GM_ASSERT(index == 0);
		return GMTextureRegisterQuery<GMTextureType::Lightmap>::Value;
	default:
		GM_ASSERT(false);
		return -1;
	}
}

void GMGLRenderer::drawTexture(GMModel* model, GMTextureType type, GMint index)
{
	D(d);
	if (d->debugConfig.get(GMDebugConfigs::DrawLightmapOnly_Bool).toBool() && type != GMTextureType::Lightmap)
		return;

	// 按照贴图类型选择纹理动画序列
	GMTextureFrames& textures = model->getShader().getTexture().getTextureFrames(type, index);

	// 获取序列中的这一帧
	ITexture* texture = getTexture(textures);
	if (texture)
	{
		// 激活动画序列
		GMint texId = activateTexture(model, (GMTextureType)type, index);
		texture->useTexture(&textures, texId);
	}
}

ITexture* GMGLRenderer::getTexture(GMTextureFrames& frames)
{
	if (frames.getFrameCount() == 0)
		return nullptr;

	if (frames.getFrameCount() == 1)
		return frames.getFrameByIndex(0);

	// 如果frameCount > 1，说明是个动画，要根据Shader的间隔来选择合适的帧
	// TODO
	GMint elapsed = GM.getGameTimeSeconds() * 1000;

	return frames.getFrameByIndex((elapsed / frames.getAnimationMs()) % frames.getFrameCount());
}

//////////////////////////////////////////////////////////////////////////
void GMGLRenderer_3D::beginModel(GMModel* model, const GMGameObject* parent)
{
	D(d);
	auto shaderProgram = GM.getGraphicEngine()->getShaderProgram();
	shaderProgram->useProgram();

	auto& desc = shaderProgram->getDesc();
	shaderProgram->setInterfaceInstance(GMGLShaderProgram::techniqueName(), getTechnique(model->getType()), GMShaderType::Vertex);
	shaderProgram->setInterfaceInstance(GMGLShaderProgram::techniqueName(), getTechnique(model->getType()), GMShaderType::Pixel);
	if (parent)
	{
		shaderProgram->setMatrix4(desc.ModelMatrix, parent->getTransform());
		shaderProgram->setMatrix4(desc.InverseTransposeModelMatrix, InverseTranspose(parent->getTransform()));
	}
	else
	{
		shaderProgram->setMatrix4(desc.ModelMatrix, Identity<GMMat4>());
		shaderProgram->setMatrix4(desc.InverseTransposeModelMatrix, Identity<GMMat4>());
	}
}

void GMGLRenderer_3D::beforeDraw(GMModel* model)
{
	D(d);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// 材质
	activateMaterial(model->getShader());

	// 应用Shader
	applyShader(model->getShader());

	// 纹理
	GM_FOREACH_ENUM_CLASS(type, GMTextureType::Ambient, GMTextureType::EndOfCommonTexture)
	{
		GMint count = GMMaxTextureCount(type);
		for (GMint i = 0; i < count; i++)
		{
			drawTexture(model, (GMTextureType)type, i);
		}
	}

	// 调试绘制
	drawDebug();
}

void GMGLRenderer_3D::afterDraw(GMModel* model)
{
	D(d);
	if (model->getShader().getBlend())
		glDepthMask(GL_TRUE);

	GM_FOREACH_ENUM_CLASS(type, GMTextureType::Ambient, GMTextureType::EndOfCommonTexture)
	{
		GMint count = GMMaxTextureCount(type);
		for (GMint i = 0; i < count; i++)
		{
			deactivateTexture((GMTextureType)type, i);
		}
	}
}

void GMGLRenderer_3D::endModel()
{
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
	D_BASE(db, Base);
	auto shaderProgram = GM.getGraphicEngine()->getShaderProgram();
	shaderProgram->setInt(GMSHADER_DEBUG_DRAW_NORMAL, db->debugConfig.get(GMDebugConfigs::DrawPolygonNormalMode).toInt());
}

//////////////////////////////////////////////////////////////////////////
void GMGLRenderer_2D::beforeDraw(GMModel* model)
{
	D(d);
	// 应用Shader
	applyShader(model->getShader());

	// 只选择环境光纹理
	GMTextureFrames& textures = model->getShader().getTexture().getTextureFrames(GMTextureType::Ambient, 0);

	// 获取序列中的这一帧
	ITexture* texture = getTexture(textures);
	if (texture)
	{
		// 激活动画序列
		GMint texId = activateTexture(model, GMTextureType::Ambient, 0);
		texture->useTexture(&textures, texId);
	}
}

//////////////////////////////////////////////////////////////////////////
void GMGLRenderer_CubeMap::beginModel(GMModel* model, const GMGameObject* parent)
{
	D(d);
	D_BASE(db, Base);
	d->cubemap = GMObject::gmobject_cast<const GMCubeMapGameObject*>(parent);
	IShaderProgram* shaderProgram = db->engine->getShaderProgram(GMShaderProgramType::CurrentShaderProgram);
	shaderProgram->useProgram();
	auto& desc = shaderProgram->getDesc();
	GM_ASSERT(model->getType() == GMModelType::CubeMap);
	shaderProgram->setInterfaceInstance(GMGLShaderProgram::techniqueName(), getTechnique(model->getType()), GMShaderType::Vertex);
	shaderProgram->setInterfaceInstance(GMGLShaderProgram::techniqueName(), getTechnique(model->getType()), GMShaderType::Pixel);
	shaderProgram->setMatrix4(desc.ModelMatrix, GMMat4(Inhomogeneous(parent->getTransform())));
	shaderProgram->setMatrix4(desc.InverseTransposeModelMatrix, GMMat4(Inhomogeneous(parent->getTransform())));
}

void GMGLRenderer_CubeMap::endModel()
{
	D(d);
	d->cubemap = nullptr;
}

void GMGLRenderer_CubeMap::beforeDraw(GMModel* model)
{
	D(d);
	D_BASE(db, Base);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	GMTexture& texture = model->getShader().getTexture();
	GMTextureFrames& frames = texture.getTextureFrames(GMTextureType::CubeMap, 0);
	ITexture* glTex = frames.getFrameByIndex(0);
	if (glTex)
	{
		IShaderProgram* shaderProgram = GM.getGraphicEngine()->getShaderProgram();
		auto& desc = shaderProgram->getDesc();

		IGraphicEngine* engine = GM.getGraphicEngine();
		// 给延迟渲染程序传入CubeMap
		{
			IShaderProgram* shader = engine->getShaderProgram(GMShaderProgramType::DeferredGeometryPassShaderProgram);
			shader->useProgram();
			shader->setInt(desc.CubeMapTextureName, GMTextureRegisterQuery<GMTextureType::CubeMap>::Value);
		}
		{
			IShaderProgram* shader = engine->getShaderProgram(GMShaderProgramType::DeferredLightPassShaderProgram);
			shader->useProgram();
			shader->setInt(desc.CubeMapTextureName, GMTextureRegisterQuery<GMTextureType::CubeMap>::Value);
		}

		// 给正向渲染程序传入CubeMap，一定要放在最后，因为CubeMap渲染本身是正向渲染
		{
			IShaderProgram* shader = engine->getShaderProgram(GMShaderProgramType::ForwardShaderProgram);
			shader->useProgram();
			shader->setInt(desc.CubeMapTextureName, GMTextureRegisterQuery<GMTextureType::CubeMap>::Value);
		}
		
		shaderProgram->useProgram();
		glTex->useTexture(&frames, GMTextureRegisterQuery<GMTextureType::CubeMap>::Value);
		db->engine->setCubeMap(glTex);
	}
}

void GMGLRenderer_CubeMap::afterDraw(GMModel* model)
{
}

void GMGLRenderer_Filter::beforeDraw(GMModel* model)
{
	applyShader(model->getShader());
	GMTextureFrames& textures = model->getShader().getTexture().getTextureFrames(GMTextureType::Ambient, 0);
	ITexture* texture = getTexture(textures);
	GM_ASSERT(texture);
	GMint texId = activateTexture(model, GMTextureType::Ambient, 0);
	texture->useTexture(&textures, texId);
}

void GMGLRenderer_Filter::afterDraw(GMModel* model)
{
}

void GMGLRenderer_Filter::beginModel(GMModel* model, const GMGameObject* parent)
{
	D(d);
	IShaderProgram* shaderProgram = d->engine->getShaderProgram(GMShaderProgramType::FilterShaderProgram);
	GM_ASSERT(shaderProgram);
	shaderProgram->useProgram();
}

void GMGLRenderer_Filter::endModel()
{
}

GMint GMGLRenderer_Filter::activateTexture(GMModel* model, GMTextureType type, GMint index)
{
	D(d);
	GMint texId = getTextureID(type, index);
	IShaderProgram* shaderProgram = d->engine->getShaderProgram(GMShaderProgramType::FilterShaderProgram);
	shaderProgram->setInt(GMSHADER_FRAMEBUFFER, texId);

	const GMShaderVariablesDesc& desc = shaderProgram->getDesc();
	bool b = shaderProgram->setInterfaceInstance(desc.FilterAttributes.Filter, desc.FilterAttributes.Types[d->engine->getCurrentFilterMode()], GMShaderType::Pixel);
	GM_ASSERT(b);
	return texId;
}