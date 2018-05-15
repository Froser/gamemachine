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
#include "gmglgbuffer.h"
#include "gmglframebuffer.h"

#ifdef max
#undef max
#endif

namespace
{
	bool g_shadowDirty = true;

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

	inline void applyShadow(const GMShaderVariablesDesc* desc, const GMShadowSourceDesc* shadowSourceDesc, IShaderProgram* shaderProgram, GMGLShadowFramebuffers* shadowFramebuffers, bool hasShadow)
	{
		static IShaderProgram* lastProgram = nullptr;
		static GMint64 lastShadowVersion = 0;
		static const GMString s_shadowInfo = GMString(desc->ShadowInfo.ShadowInfo) + ".";
		static std::string s_position = (s_shadowInfo + desc->ShadowInfo.Position).toStdString();
		static std::string s_matrix = (s_shadowInfo + desc->ShadowInfo.ShadowMatrix).toStdString();
		static std::string s_width = (s_shadowInfo + desc->ShadowInfo.ShadowMapWidth).toStdString();
		static std::string s_height = (s_shadowInfo + desc->ShadowInfo.ShadowMapWidth).toStdString();
		static std::string s_biasMin = (s_shadowInfo + desc->ShadowInfo.BiasMin).toStdString();
		static std::string s_biasMax = (s_shadowInfo + desc->ShadowInfo.BiasMax).toStdString();
		static std::string s_hasShadow = (s_shadowInfo + desc->ShadowInfo.HasShadow).toStdString();
		static std::string s_shadowMap = (s_shadowInfo + desc->ShadowInfo.ShadowMap).toStdString();

		if (hasShadow)
		{
			if (g_shadowDirty || lastShadowVersion != shadowSourceDesc->currentVersion)
			{
				lastProgram = shaderProgram;
				lastShadowVersion = shadowSourceDesc->currentVersion;

				shaderProgram->setBool(s_hasShadow.c_str(), 1);
				const GMCamera& camera = shadowSourceDesc->camera;
				GMFloat4 viewPosition;
				shadowSourceDesc->position.loadFloat4(viewPosition);
				shaderProgram->setVec4(s_position.c_str(), viewPosition);
				shaderProgram->setMatrix4(s_matrix.c_str(), camera.getViewMatrix() * camera.getProjectionMatrix());
				shaderProgram->setFloat(s_biasMin.c_str(), shadowSourceDesc->biasMin);
				shaderProgram->setFloat(s_biasMax.c_str(), shadowSourceDesc->biasMax);

				shaderProgram->setInt(s_width.c_str(), shadowFramebuffers->getShadowMapWidth());
				shaderProgram->setInt(s_height.c_str(), shadowFramebuffers->getShadowMapHeight());
				shaderProgram->setInt(s_shadowMap.c_str(), GMTextureRegisterQuery<GMTextureType::ShadowMap>::Value);
				g_shadowDirty = false;
			}
		}
		else
		{
			shaderProgram->setBool(s_hasShadow.c_str(), 0);
		}
	}


	GM_PRIVATE_OBJECT(GMGLWhiteTexture)
	{
		GMuint textureId = 0;
	};

	class GMGLWhiteTexture : public ITexture
	{
		DECLARE_PRIVATE(GMGLWhiteTexture)

	public:
		GMGLWhiteTexture() = default;

		~GMGLWhiteTexture()
		{
			D(d);
			glDeleteTextures(1, &d->textureId);
		}

		virtual void init() override
		{
			D(d);
			static GMbyte texData[] = { 0xFF, 0xFF, 0xFF, 0xFF };
			glGenTextures(1, &d->textureId);
			glBindTexture(GL_TEXTURE_2D, d->textureId);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		virtual void bindSampler(GMTextureSampler* sampler)
		{
			D(d);
			glBindTexture(GL_TEXTURE_2D, d->textureId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		virtual void useTexture(GMint textureIndex) override
		{
			D(d);
			glActiveTexture(GL_TEXTURE0 + textureIndex);
			glBindTexture(GL_TEXTURE_2D, d->textureId);
		}
	};

	ITexture* getWhiteTexture()
	{
		static bool s_inited = false;
		static GMGLWhiteTexture s_texture;
		if (!s_inited)
		{
			s_texture.init();
			s_inited = true;
		}
		return &s_texture;
	}
}

void GMGammaHelper::setGamma(const GMShaderVariablesDesc* desc, GMGraphicEngine* engine, IShaderProgram* shaderProgram)
{
	bool needGammaCorrection = engine->needGammaCorrection();
	shaderProgram->setBool(desc->GammaCorrection.GammaCorrection, needGammaCorrection);
	if (needGammaCorrection)
	{
		GMfloat gamma = engine->getGammaValue();
		if (gamma != m_gamma)
		{
			shaderProgram->setFloat(desc->GammaCorrection.GammaInvValue, gamma);
			m_gamma = gamma;
		}
	}
}

GMGLRenderer::GMGLRenderer()
{
	D(d);
	d->engine = gm_cast<GMGLGraphicEngine*>(GM.getGraphicEngine());
	d->debugConfig = GM.getConfigs().getConfig(GMConfigs::Debug).asDebugConfig();
}

void GMGLRenderer::draw(GMModel* model)
{
	D(d);
	glBindVertexArray(model->getModelBuffer()->getMeshBuffer().arrayId);
	applyStencil(*d->engine);
	prepareScreenInfo(getShaderProgram());
	beforeDraw(model);
	GLenum mode = d->debugConfig.get(GMDebugConfigs::DrawPolygonsAsLine_Bool).toBool() ? GL_LINE_LOOP : getMode(model->getPrimitiveTopologyMode());
	GM_ASSERT(model->getVerticesCount() < std::numeric_limits<GMuint>::max());
	if (model->getDrawMode() == GMModelDrawMode::Vertex)
		glDrawArrays(mode, 0, (GLsizei)model->getVerticesCount());
	else
		glDrawElements(mode, (GLsizei)model->getVerticesCount(), GL_UNSIGNED_INT, 0);
	afterDraw(model);
	glBindVertexArray(0);
}

void GMGLRenderer::activateTextureTransform(GMModel* model, GMTextureType type)
{
	const GMShaderVariablesDesc* desc = getVariablesDesc();
	GM_ASSERT(desc);
	static const std::string u_scrolls_affix = (GMString(".") + desc->TextureAttributes.OffsetX).toStdString();
	static const std::string u_scrollt_affix = (GMString(".") + desc->TextureAttributes.OffsetY).toStdString();
	static const std::string u_scales_affix = (GMString(".") + desc->TextureAttributes.ScaleX).toStdString();
	static const std::string u_scalet_affix = (GMString(".") + desc->TextureAttributes.ScaleY).toStdString();

	auto shaderProgram = getShaderProgram();
	const char* uniform = getTextureUniformName(desc, type);

	static char u_scrolls[GMGL_MAX_UNIFORM_NAME_LEN],
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

	if (model)
		model->getShader().getTextureList().getTextureSampler(type).applyTexMode(GM.getGameTimeSeconds(), applyCallback);
}

GMint GMGLRenderer::activateTexture(GMModel* model, GMTextureType type)
{
	const GMShaderVariablesDesc* desc = getVariablesDesc();
	GM_ASSERT(desc);
	static const std::string u_tex_enabled = (GMString(".") + desc->TextureAttributes.Enabled).toStdString();
	static const std::string u_tex_texture = (GMString(".") + desc->TextureAttributes.Texture).toStdString();

	GMint idx = (GMint)type;
	GMint texId = getTextureID(type);

	auto shaderProgram = getShaderProgram();
	const char* uniform = getTextureUniformName(desc, type);
	static char u_texture[GMGL_MAX_UNIFORM_NAME_LEN], u_enabled[GMGL_MAX_UNIFORM_NAME_LEN];
	combineUniform(u_texture, uniform, u_tex_texture.c_str());
	combineUniform(u_enabled, uniform, u_tex_enabled.c_str());
	shaderProgram->setInt(u_texture, texId);
	shaderProgram->setInt(u_enabled, 1);

	activateTextureTransform(model, type);
	return texId;
}

void GMGLRenderer::deactivateTexture(GMTextureType type)
{
	const GMShaderVariablesDesc* desc = getVariablesDesc();
	GM_ASSERT(desc);
	static const std::string u_tex_enabled = (GMString(".") + desc->TextureAttributes.Enabled).toStdString();
	GMint texId = getTextureID(type);

	auto shaderProgram = getShaderProgram();
	GMint idx = (GMint)type;
	const char* uniform = getTextureUniformName(desc, type);
	static char u[GMGL_MAX_UNIFORM_NAME_LEN];
	combineUniform(u, uniform, u_tex_enabled.c_str());
	shaderProgram->setInt(u, 0);
}

GMint GMGLRenderer::getTextureID(GMTextureType type)
{
	switch (type)
	{
	case GMTextureType::Ambient:
		return GMTextureRegisterQuery<GMTextureType::Ambient>::Value;
	case GMTextureType::Diffuse:
		return GMTextureRegisterQuery<GMTextureType::Diffuse>::Value;
	case GMTextureType::Specular:
		return GMTextureRegisterQuery<GMTextureType::Specular>::Value;
	case GMTextureType::NormalMap:
		return GMTextureRegisterQuery<GMTextureType::NormalMap>::Value;
	case GMTextureType::Albedo:
		return GMTextureRegisterQuery<GMTextureType::Albedo>::Value;
	case GMTextureType::MetallicRoughnessAO:
		return GMTextureRegisterQuery<GMTextureType::MetallicRoughnessAO>::Value;
	case GMTextureType::Lightmap:
		return GMTextureRegisterQuery<GMTextureType::Lightmap>::Value;
	default:
		GM_ASSERT(false);
		return -1;
	}
}

bool GMGLRenderer::drawTexture(GMModel* model, GMTextureType type)
{
	D(d);
	if (d->debugConfig.get(GMDebugConfigs::DrawLightmapOnly_Bool).toBool() && type != GMTextureType::Lightmap)
		return true;

	// 按照贴图类型选择纹理动画序列
	GMTextureSampler& sampler = model->getShader().getTextureList().getTextureSampler(type);

	// 获取序列中的这一帧
	ITexture* texture = getTexture(sampler);
	if (texture)
	{
		// 激活动画序列
		GMint texId = activateTexture(model, (GMTextureType)type);
		texture->bindSampler(&sampler);
		texture->useTexture(texId);
		return true;
	}
	return false;
}

ITexture* GMGLRenderer::getTexture(GMTextureSampler& frames)
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

void GMGLRenderer::updateCameraMatrices(IShaderProgram* shaderProgram)
{
	static IShaderProgram* s_lastShaderProgram = nullptr;

	GMCamera& camera = GM.getCamera();
	if (s_lastShaderProgram != shaderProgram || camera.isDirty())
	{
		const GMMat4& viewMatrix = camera.getViewMatrix();
		const GMCameraLookAt& lookAt = camera.getLookAt();
		GMFloat4 vec;
		lookAt.position.loadFloat4(vec);

		auto& desc = shaderProgram->getDesc();
		shaderProgram->setVec4(desc.ViewPosition, vec);
		shaderProgram->setMatrix4(desc.ViewMatrix, camera.getViewMatrix());
		shaderProgram->setMatrix4(desc.InverseViewMatrix, camera.getInverseViewMatrix());
		shaderProgram->setMatrix4(desc.ProjectionMatrix, camera.getProjectionMatrix());
		s_lastShaderProgram = shaderProgram;
		camera.cleanDirty();
	}
}

void GMGLRenderer::prepareScreenInfo(IShaderProgram* shaderProgram)
{
	static IShaderProgram* s_lastShaderProgram = nullptr;
	if (s_lastShaderProgram != shaderProgram) //或者窗口属性发生改变
	{
		const GMGameMachineRunningStates& states = GM.getGameMachineRunningStates();
		auto& desc = shaderProgram->getDesc();
		shaderProgram->setInt(desc.ScreenInfoAttributes.Multisampling, states.sampleCount > 1);
		shaderProgram->setInt(desc.ScreenInfoAttributes.ScreenWidth, states.renderRect.width);
		shaderProgram->setInt(desc.ScreenInfoAttributes.ScreenHeight, states.renderRect.height);
		s_lastShaderProgram = shaderProgram;
	}
}

void GMGLRenderer::dirtyShadowMapAttributes()
{
	g_shadowDirty = true;
}

//////////////////////////////////////////////////////////////////////////
void GMGLRenderer_3D::beginModel(GMModel* model, const GMGameObject* parent)
{
	D(d);
	D_BASE(db, GMGLRenderer);
	auto shaderProgram = getShaderProgram();
	shaderProgram->useProgram();
	updateCameraMatrices(shaderProgram);

	static const GMShaderVariablesDesc* desc = getVariablesDesc();
	if (parent)
	{
		shaderProgram->setMatrix4(desc->ModelMatrix, parent->getTransform());
		shaderProgram->setMatrix4(desc->InverseTransposeModelMatrix, InverseTranspose(parent->getTransform()));
	}
	else
	{
		shaderProgram->setMatrix4(desc->ModelMatrix, Identity<GMMat4>());
		shaderProgram->setMatrix4(desc->InverseTransposeModelMatrix, Identity<GMMat4>());
	}

	GMGeometryPassingState state = db->engine->getGBuffer()->getGeometryPassingState();
	if (state != GMGeometryPassingState::PassingGeometry)
	{
		shaderProgram->setInterfaceInstance(
			GMGLShaderProgram::techniqueName(),
			getTechnique(model->getType()),
			GMShaderType::Vertex);
		shaderProgram->setInterfaceInstance(
			GMGLShaderProgram::techniqueName(),
			getTechnique(model->getType()),
			GMShaderType::Pixel);
		db->engine->activateLights(this);
	}

	const GMShadowSourceDesc& shadowSourceDesc = db->engine->getShadowSourceDesc();
	if (shadowSourceDesc.type != GMShadowSourceDesc::NoShadow)
	{
		GMGLShadowFramebuffers* shadowFramebuffers = gm_cast<GMGLShadowFramebuffers*>(db->engine->getShadowMapFramebuffers());
		shadowFramebuffers->getShadowMapTexture()->useTexture(0);
		applyShadow(desc, &shadowSourceDesc, shaderProgram, shadowFramebuffers, true);
	}
	else
	{
		applyShadow(desc, nullptr, shaderProgram, nullptr, false);
	}

	db->gammaHelper.setGamma(desc, db->engine, shaderProgram);
}

void GMGLRenderer_3D::beforeDraw(GMModel* model)
{
	D(d);
	D_BASE(db, GMGLRenderer);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// 材质
	activateMaterial(model->getShader());

	// 应用Shader
	applyShader(model->getShader());

	// 纹理
	GM_FOREACH_ENUM_CLASS(type, GMTextureType::Ambient, GMTextureType::EndOfCommonTexture)
	{
		if (!drawTexture(model, (GMTextureType)type))
		{
			if (type == GMTextureType::Ambient ||
				type == GMTextureType::Diffuse ||
				type == GMTextureType::Specular ||
				type == GMTextureType::Lightmap
				)
			{
				GMint texId = activateTexture(nullptr, (GMTextureType)type);
				getWhiteTexture()->useTexture(texId);
			}
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
		deactivateTexture((GMTextureType)type);
	}
}

IShaderProgram* GMGLRenderer_3D::getShaderProgram()
{
	D_BASE(d, Base);
	GMGeometryPassingState s = d->engine->getGBuffer()->getGeometryPassingState();
	if (s == GMGeometryPassingState::PassingGeometry)
		return d->engine->getShaderProgram(GMShaderProgramType::DeferredGeometryPassShaderProgram);

	return d->engine->getShaderProgram(GMShaderProgramType::ForwardShaderProgram);
}

void GMGLRenderer_3D::endModel()
{
}

void GMGLRenderer_3D::activateMaterial(const GMShader& shader)
{
	D(d);
	auto shaderProgram = getShaderProgram();
	auto desc = getVariablesDesc();
	static const std::string GMSHADER_MATERIAL_KA = std::string(desc->MaterialName) + "." + desc->MaterialAttributes.Ka;
	static const std::string GMSHADER_MATERIAL_KD = std::string(desc->MaterialName) + "." + desc->MaterialAttributes.Kd;
	static const std::string GMSHADER_MATERIAL_KS = std::string(desc->MaterialName) + "." + desc->MaterialAttributes.Ks;
	static const std::string GMSHADER_MATERIAL_SHININESS = std::string(desc->MaterialName) + "." + desc->MaterialAttributes.Shininess;
	static const std::string GMSHADER_MATERIAL_REFRACTIVITY = std::string(desc->MaterialName) + "." + desc->MaterialAttributes.Refreactivity;

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
	auto shaderProgram = getShaderProgram();
	shaderProgram->setInt(GMSHADER_DEBUG_DRAW_NORMAL, db->debugConfig.get(GMDebugConfigs::DrawPolygonNormalMode).toInt());
}

//////////////////////////////////////////////////////////////////////////
void GMGLRenderer_2D::beforeDraw(GMModel* model)
{
	D(d);
	// 应用Shader
	applyShader(model->getShader());

	// 只选择环境光纹理
	GMTextureSampler& sampler = model->getShader().getTextureList().getTextureSampler(GMTextureType::Ambient);

	// 获取序列中的这一帧
	ITexture* texture = getTexture(sampler);
	if (texture)
	{
		// 激活动画序列
		GMint texId = activateTexture(model, GMTextureType::Ambient);
		texture->bindSampler(&sampler);
		texture->useTexture(texId);
	}
}

//////////////////////////////////////////////////////////////////////////
void GMGLRenderer_CubeMap::beginModel(GMModel* model, const GMGameObject* parent)
{
	IShaderProgram* shaderProgram = getShaderProgram();
	shaderProgram->useProgram();
	updateCameraMatrices(shaderProgram);

	auto desc = getVariablesDesc();
	shaderProgram->setMatrix4(desc->ModelMatrix, GMMat4(Inhomogeneous(parent->getTransform())));
}

void GMGLRenderer_CubeMap::endModel()
{
}

void GMGLRenderer_CubeMap::beforeDraw(GMModel* model)
{
	D(d);
	D_BASE(db, Base);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	GMTextureList& texture = model->getShader().getTextureList();
	GMTextureSampler& sampler = texture.getTextureSampler(GMTextureType::CubeMap);
	ITexture* glTex = sampler.getFrameByIndex(0);
	if (glTex)
	{
		IShaderProgram* shaderProgram = getShaderProgram();
		auto desc = getVariablesDesc();
		GM_ASSERT(model->getType() == GMModelType::CubeMap);
		shaderProgram->setInterfaceInstance(GMGLShaderProgram::techniqueName(), getTechnique(model->getType()), GMShaderType::Vertex);
		shaderProgram->setInterfaceInstance(GMGLShaderProgram::techniqueName(), getTechnique(model->getType()), GMShaderType::Pixel);
		shaderProgram->setInt(desc->CubeMapTextureName, GMTextureRegisterQuery<GMTextureType::CubeMap>::Value);
		glTex->bindSampler(&sampler);
		glTex->useTexture(GMTextureRegisterQuery<GMTextureType::CubeMap>::Value);
		db->engine->setCubeMap(glTex);
	}
}

void GMGLRenderer_CubeMap::afterDraw(GMModel* model)
{
}

void GMGLRenderer_Filter::beforeDraw(GMModel* model)
{
	applyShader(model->getShader());
	GMTextureSampler& sampler = model->getShader().getTextureList().getTextureSampler(GMTextureType::Ambient);
	ITexture* texture = getTexture(sampler);
	GM_ASSERT(texture);
	GMint texId = activateTexture(model, GMTextureType::Ambient);
	texture->bindSampler(&sampler);
	texture->useTexture(texId);
}

void GMGLRenderer_Filter::afterDraw(GMModel* model)
{
}

void GMGLRenderer_Filter::beginModel(GMModel* model, const GMGameObject* parent)
{
	D(d);
	D_BASE(db, Base);
	IShaderProgram* shaderProgram = getShaderProgram();
	GM_ASSERT(shaderProgram);
	shaderProgram->useProgram();

	static const GMShaderVariablesDesc* desc = getVariablesDesc();
	if (d->state.HDR != db->engine->needHDR() || d->state.toneMapping != db->engine->getToneMapping())
	{
		d->state.HDR = db->engine->needHDR();
		d->state.toneMapping = db->engine->getToneMapping();
		if (d->state.HDR)
		{
			db->gammaHelper.setGamma(desc, db->engine, shaderProgram);
			setHDR(shaderProgram);
		}
		else
		{
			shaderProgram->setBool(desc->HDR.HDR, false);
		}
	}
}

void GMGLRenderer_Filter::setHDR(IShaderProgram* shaderProgram)
{
	static const GMShaderVariablesDesc* desc = getVariablesDesc();
	D(d);
	shaderProgram->setBool(desc->HDR.HDR, true);
	shaderProgram->setInt(desc->HDR.ToneMapping, d->state.toneMapping);
}

void GMGLRenderer_Filter::endModel()
{
}

IShaderProgram* GMGLRenderer_Filter::getShaderProgram()
{
	D_BASE(db, GMGLRenderer);
	return db->engine->getShaderProgram(GMShaderProgramType::FilterShaderProgram);
}

GMint GMGLRenderer_Filter::activateTexture(GMModel* model, GMTextureType type)
{
	D(d);
	D_BASE(db, Base);
	GMint texId = getTextureID(type);
	IShaderProgram* shaderProgram = getShaderProgram();
	shaderProgram->setInt(GMSHADER_FRAMEBUFFER, texId);

	const GMShaderVariablesDesc* desc = getVariablesDesc();
	bool b = shaderProgram->setInterfaceInstance(desc->FilterAttributes.Filter, desc->FilterAttributes.Types[db->engine->getCurrentFilterMode()], GMShaderType::Pixel);
	GM_ASSERT(b);
	return texId;
}

IShaderProgram* GMGLRenderer_LightPass::getShaderProgram()
{
	D_BASE(db, GMGLRenderer);
	return db->engine->getShaderProgram(GMShaderProgramType::DeferredLightPassShaderProgram);
}

void GMGLRenderer_LightPass::endModel()
{
}

void GMGLRenderer_LightPass::beginModel(GMModel* model, const GMGameObject* parent)
{
	D_BASE(d, GMGLRenderer);
	IShaderProgram* shaderProgram = getShaderProgram();
	shaderProgram->useProgram();
	updateCameraMatrices(shaderProgram);

	static const GMShaderVariablesDesc* desc = getVariablesDesc();
	const GMShadowSourceDesc& shadowSourceDesc = d->engine->getShadowSourceDesc();
	if (shadowSourceDesc.type != GMShadowSourceDesc::NoShadow)
	{
		GMGLShadowFramebuffers* shadowFramebuffers = gm_cast<GMGLShadowFramebuffers*>(d->engine->getShadowMapFramebuffers());
		shadowFramebuffers->getShadowMapTexture()->useTexture(0);
		applyShadow(desc, &shadowSourceDesc, shaderProgram, shadowFramebuffers, true);
	}
	else
	{
		applyShadow(desc, nullptr, shaderProgram, nullptr, false);
	}

	d->gammaHelper.setGamma(desc, d->engine, shaderProgram);
}

void GMGLRenderer_LightPass::afterDraw(GMModel* model)
{
}

void GMGLRenderer_LightPass::beforeDraw(GMModel* model)
{
	D_BASE(d, GMGLRenderer);
	IShaderProgram* shaderProgram = getShaderProgram();
	d->engine->activateLights(this);
	IGBuffer* gBuffer = d->engine->getGBuffer();
	IFramebuffers* gBufferFramebuffers = gBuffer->getGeometryFramebuffers();
	GMsize_t cnt = gBufferFramebuffers->count();
	for (GMsize_t i = 0; i < cnt; ++i)
	{
		ITexture* texture = gBufferFramebuffers->getFramebuffer(i)->getTexture();
		const GMsize_t textureIndex = (GMTextureRegisterQuery<GMTextureType::GeometryPasses>::Value + i);
		GM_ASSERT(textureIndex < std::numeric_limits<GMuint>::max());
		shaderProgram->setInt(GMGLGBuffer::GBufferGeometryUniformNames()[i].c_str(), (GMuint)textureIndex);
		texture->useTexture((GMuint)textureIndex);
	}

	ITexture* cubeMap = d->engine->getCubeMap();
	if (cubeMap)
	{
		const GMsize_t id = GMTextureRegisterQuery<GMTextureType::GeometryPasses>::Value + 1 + cnt;
		GM_ASSERT(id < std::numeric_limits<GMuint>::max());
		shaderProgram->setInt(getVariablesDesc()->CubeMapTextureName, (GMuint)id);
		cubeMap->useTexture((GMuint)id);
	}
}

void GMGLRenderer_3D_Shadow::beginModel(GMModel* model, const GMGameObject* parent)
{
	D_BASE(d, Base);
	IShaderProgram* shaderProgram = getShaderProgram();
	shaderProgram->useProgram();
	static const GMShaderVariablesDesc* desc = getVariablesDesc();
	if (parent)
		shaderProgram->setMatrix4(desc->ModelMatrix, parent->getTransform());
	else
		shaderProgram->setMatrix4(desc->ModelMatrix, Identity<GMMat4>());

	GMGLShadowFramebuffers* shadowFramebuffers = gm_cast<GMGLShadowFramebuffers*>(d->engine->getShadowMapFramebuffers());
	applyShadow(desc, &d->engine->getShadowSourceDesc(), shaderProgram, shadowFramebuffers, true);

	bool b = shaderProgram->setInterfaceInstance(
		GMGLShaderProgram::techniqueName(),
		"GM_Shadow",
		GMShaderType::Vertex);
	GM_ASSERT(b);
	b = shaderProgram->setInterfaceInstance(
		GMGLShaderProgram::techniqueName(),
		"GM_Shadow",
		GMShaderType::Pixel);
	GM_ASSERT(b);
}