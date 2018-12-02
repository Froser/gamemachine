#include "stdafx.h"
#include <GL/glew.h>
#include "gmgltechniques.h"
#include "gmgl/gmglgraphic_engine.h"
#include "gmgl/shader_constants.h"
#include "gmgl/gmgltexture.h"
#include "gmengine/gmgameworld.h"
#include <linearmath.h>
#include "foundation/gamemachine.h"
#include "foundation/utilities/utilities.h"
#include "gmglgbuffer.h"
#include "gmglframebuffer.h"

namespace
{
	bool g_shadowDirty = true;

	// variable index getter (with shaderProgram)
#define VI_SP(d, name, shaderProgram, indexBank) \
	getVariableIndex(shaderProgram, d->indexBank[verifyIndicesContainer(d->indexBank, shaderProgram)]. name, GM_VariablesDesc. ## name)

#define VI(name) VI_SP(d, name, shaderProgram, indexBank)

	// variable index from base
#define VI_B(name) VI_SP(db, name, shaderProgram, indexBank)

	// variable index from name
#define VI_C(d, name, str) \
	getVariableIndex(shaderProgram, d->indexBank[verifyIndicesContainer(d->indexBank, shaderProgram)]. name, str)
#define VI_N(name, str) VI_C(d, name, str)
#define VI_NB(name, str) VI_C(db, name, str)

	// global variables
	struct
	{
		IShaderProgram* lastShaderProgram_camera = nullptr;
		IShaderProgram* lastShaderProgram_screenInfo = nullptr;
	} s_shaderCaches;

	inline GLenum toStencilOp(GMStencilOptions::GMStencilOp stencilOptions)
	{
		switch (stencilOptions)
		{
		case GMStencilOptions::Keep:
			return GL_KEEP;
		case GMStencilOptions::Zero:
			return GL_ZERO;
		case GMStencilOptions::Replace:
			return GL_REPLACE;
		default:
			GM_ASSERT(false);
			return GL_KEEP;
		}
	}

	inline void prepareStencil(GMGLGraphicEngine& engine)
	{
		// 应用模板
		GLenum compareFunc = GL_ALWAYS;
		auto& stencilOptions = engine.getStencilOptions();
		switch (stencilOptions.compareFunc)
		{
		case GMStencilOptions::Equal:
			compareFunc = GL_EQUAL;
			break;
		case GMStencilOptions::NotEqual:
			compareFunc = GL_NOTEQUAL;
			break;
		case GMStencilOptions::Never:
			compareFunc = GL_NEVER;
			break;
		case GMStencilOptions::Always:
			compareFunc = GL_ALWAYS;
			break;
		case GMStencilOptions::Less:
			compareFunc = GL_LESS;
			break;
		case GMStencilOptions::LessEqual:
			compareFunc = GL_LEQUAL;
			break;
		case GMStencilOptions::Greater:
			compareFunc = GL_GREATER;
			break;
		case GMStencilOptions::GreaterEqual:
			compareFunc = GL_GEQUAL;
			break;
		default:
			GM_ASSERT(false);
		}

		glStencilFunc(compareFunc, 1, 0xFF);
		glStencilMask(stencilOptions.writeMask);
		glStencilOp(toStencilOp(stencilOptions.stencilFailedOp), toStencilOp(stencilOptions.stencilDepthFailedOp), toStencilOp(stencilOptions.stencilPassOp));
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
			return GL_LINES;
		default:
			GM_ASSERT(false);
			return GL_TRIANGLES;
		}
	}

	const GMString& getTechnique(GMModelType type)
	{
		static const GMString s_2D = L"GM_Model2D";
		static const GMString s_3D = L"GM_Model3D";
		static const GMString s_text = L"GM_Text";
		static const GMString s_cubemap = L"GM_CubeMap";
		static const GMString s_particle = L"GM_Particle";
		static const GMString s_custom = L"GM_Custom";

		switch (type)
		{
		case GMModelType::Model2D:
			return s_2D;
		case GMModelType::Model3D:
			return s_3D;
		case GMModelType::Text:
			return s_text;
		case GMModelType::CubeMap:
			return s_cubemap;
		case GMModelType::Particle:
			return s_particle;
		case GMModelType::Custom:
			return s_custom;
		default:
			GM_ASSERT(false);
			return s_2D;
		}
	}

	void applyShadow(GMGLTechnique* tech, const GMShadowSourceDesc* shadowSourceDesc, IShaderProgram* shaderProgram, GMGLShadowFramebuffers* shadowFramebuffers, bool hasShadow)
	{
		D_OF(d, tech);
		static IShaderProgram* lastProgram = nullptr;
		static GMint64 lastShadowVersion = 0;
		static const GMString s_shadowInfo = GMString(GM_VariablesDesc.ShadowInfo.ShadowInfo) + L".";
		static const GMString s_position = s_shadowInfo + GM_VariablesDesc.ShadowInfo.Position;
		static const GMString s_matrix = s_shadowInfo + GM_VariablesDesc.ShadowInfo.ShadowMatrix;
		static const GMString s_width = s_shadowInfo + GM_VariablesDesc.ShadowInfo.ShadowMapWidth;
		static const GMString s_height = s_shadowInfo + GM_VariablesDesc.ShadowInfo.ShadowMapHeight;
		static const GMString s_biasMin = s_shadowInfo + GM_VariablesDesc.ShadowInfo.BiasMin;
		static const GMString s_biasMax = s_shadowInfo + GM_VariablesDesc.ShadowInfo.BiasMax;
		static const GMString s_hasShadow = s_shadowInfo + GM_VariablesDesc.ShadowInfo.HasShadow;
		static const GMString s_shadowMap = s_shadowInfo + GM_VariablesDesc.ShadowInfo.ShadowMap;

		if (hasShadow)
		{
			if (g_shadowDirty || lastShadowVersion != shadowSourceDesc->version)
			{
				lastProgram = shaderProgram;
				lastShadowVersion = shadowSourceDesc->version;

				shaderProgram->setBool(VI_N(ShadowInfo.HasShadow, s_hasShadow), 1);
				const GMCamera& camera = shadowSourceDesc->camera;
				GMFloat4 viewPosition;
				shadowSourceDesc->position.loadFloat4(viewPosition);
				shaderProgram->setVec4(VI_N(ShadowInfo.Position, s_position), viewPosition);
				shaderProgram->setMatrix4(VI_N(ShadowInfo.ShadowMatrix, s_matrix), camera.getViewMatrix() * camera.getProjectionMatrix());
				shaderProgram->setFloat(VI_N(ShadowInfo.BiasMin, s_biasMin), shadowSourceDesc->biasMin);
				shaderProgram->setFloat(VI_N(ShadowInfo.BiasMax, s_biasMax), shadowSourceDesc->biasMax);

				shaderProgram->setInt(VI_N(ShadowInfo.ShadowMapWidth, s_width), shadowFramebuffers->getShadowMapWidth());
				shaderProgram->setInt(VI_N(ShadowInfo.ShadowMapHeight, s_height), shadowFramebuffers->getShadowMapHeight());
				shaderProgram->setInt(VI_N(ShadowInfo.ShadowMap, s_shadowMap), GMTextureRegisterQuery<GMTextureType::ShadowMap>::Value);
				g_shadowDirty = false;
			}
		}
		else
		{
			shaderProgram->setBool(VI_N(ShadowInfo.HasShadow, s_hasShadow), 0);
		}
	}

	GMTextureAsset createWhiteTexture(const IRenderContext* context)
	{
		GMTextureAsset texture;
		GM.getFactory()->createWhiteTexture(context, texture);
		return texture;
	}
}

void GMGammaHelper::setGamma(GMGLTechnique* tech, GMGraphicEngine* engine, IShaderProgram* shaderProgram)
{
	D_OF(d, tech);
	shaderProgram->setBool(VI(GammaCorrection.GammaCorrection), engine->needGammaCorrection());
	GMfloat gamma = engine->getGammaValue();
	if (gamma != m_gamma)
	{
		shaderProgram->setFloat(VI(GammaCorrection.GammaValue), gamma);
		shaderProgram->setFloat(VI(GammaCorrection.GammaInvValue), 1.f / gamma);
		m_gamma = gamma;
	}
}

GMGLTechnique::GMGLTechnique(const IRenderContext* context)
{
	D(d);
	d->context = context;
	d->engine = gm_cast<GMGLGraphicEngine*>(d->context->getEngine());
	d->debugConfig = GM.getConfigs().getConfig(GMConfigs::Debug).asDebugConfig();
}

void GMGLTechnique::draw(GMModel* model)
{
	D(d);
	GMGLBeginGetErrorsAndCheck();
	glBindVertexArray(model->getModelBuffer()->getMeshBuffer().arrayId);

	prepareStencil(*d->engine);
	prepareScreenInfo(getShaderProgram());
	beforeDraw(model);
	startDraw(model);
	afterDraw(model);

	glBindVertexArray(0);
	GMGLEndGetErrorsAndCheck();
}

void GMGLTechnique::beginScene(GMScene* scene)
{
	D(d);
	d->techContext.currentScene = scene;
}

void GMGLTechnique::endScene()
{
	D(d);
	d->techContext.currentScene = nullptr;
}

void GMGLTechnique::beginModel(GMModel* model, const GMGameObject* parent)
{
	D(d);
	d->techContext.currentModel = model;
	auto shaderProgram = getShaderProgram();
	shaderProgram->useProgram();

	// We must init this uniform. Otherwise some X11 (Ubuntu) Windows will be abnormal.
	shaderProgram->setInt(VI(CubeMapTextureName), GMTextureRegisterQuery<GMTextureType::CubeMap>::Value);

	// 设置顶点颜色运算方式
	shaderProgram->setInt(VI(ColorVertexOp), static_cast<GMint32>(model->getShader().getVertexColorOp()));

	// 骨骼动画
	GM_ASSERT(d->techContext.currentScene);
	if (d->techContext.currentScene->hasAnimation())
	{
		shaderProgram->setInt(VI(UseBoneAnimation), 1);
		updateBoneTransforms(shaderProgram, model);
	}
	else
	{
		shaderProgram->setInt(VI(UseBoneAnimation), 0);
	}
}

void GMGLTechnique::endModel()
{
	D(d);
	d->techContext.currentModel = nullptr;
}

void GMGLTechnique::activateTextureTransform(GMModel* model, GMTextureType type)
{
	static const GMString u_scrolls_affix = L"." + GM_VariablesDesc.TextureAttributes.OffsetX;
	static const GMString u_scrollt_affix = L"." + GM_VariablesDesc.TextureAttributes.OffsetY;
	static const GMString u_scales_affix = L"." + GM_VariablesDesc.TextureAttributes.ScaleX;
	static const GMString u_scalet_affix = L"." + GM_VariablesDesc.TextureAttributes.ScaleY;

	D(d);
	auto shaderProgram = getShaderProgram();
	GMsize_t index = verifyIndicesContainer(d->textureTransformIndices, shaderProgram);

	GMsize_t typeIndex = (GMsize_t)type;
	if (!d->textureTransformIndices[index].ScrollS[typeIndex])
		d->textureTransformIndices[index].ScrollS[typeIndex] = shaderProgram->getIndex(getTextureUniformName(type) + u_scrolls_affix);
	if (!d->textureTransformIndices[index].ScrollT[typeIndex])
		d->textureTransformIndices[index].ScrollT[typeIndex] = shaderProgram->getIndex(getTextureUniformName(type) + u_scrollt_affix);
	if (!d->textureTransformIndices[index].ScaleS[typeIndex])
		d->textureTransformIndices[index].ScaleS[typeIndex] = shaderProgram->getIndex(getTextureUniformName(type) + u_scales_affix);
	if (!d->textureTransformIndices[index].ScaleT[typeIndex])
		d->textureTransformIndices[index].ScaleT[typeIndex] = shaderProgram->getIndex(getTextureUniformName(type) + u_scalet_affix);

	shaderProgram->setFloat(d->textureTransformIndices[index].ScrollS[typeIndex], 0.f);
	shaderProgram->setFloat(d->textureTransformIndices[index].ScrollT[typeIndex], 0.f);
	shaderProgram->setFloat(d->textureTransformIndices[index].ScaleS[typeIndex], 1.f);
	shaderProgram->setFloat(d->textureTransformIndices[index].ScaleT[typeIndex], 1.f);

	auto applyCallback = [&](GMS_TextureTransformType type, Pair<GMfloat, GMfloat>&& args) {
		if (type == GMS_TextureTransformType::Scale)
		{
			shaderProgram->setFloat(d->textureTransformIndices[index].ScaleS[typeIndex], args.first);
			shaderProgram->setFloat(d->textureTransformIndices[index].ScaleT[typeIndex], args.second);
		}
		else if (type == GMS_TextureTransformType::Scroll)
		{
			shaderProgram->setFloat(d->textureTransformIndices[index].ScrollS[typeIndex], args.first);
			shaderProgram->setFloat(d->textureTransformIndices[index].ScrollT[typeIndex], args.second);
		}
		else
		{
			GM_ASSERT(false);
		}
	};

	if (model)
		model->getShader().getTextureList().getTextureSampler(type).applyTexMode(GM.getRunningStates().elapsedTime, applyCallback);
}

GMint32 GMGLTechnique::activateTexture(GMModel* model, GMTextureType type)
{
	D(d);
	static const GMString u_tex_enabled = L"." + GM_VariablesDesc.TextureAttributes.Enabled;
	static const GMString u_tex_texture = L"." + GM_VariablesDesc.TextureAttributes.Texture;

	GMint32 idx = (GMint32)type;
	GMint32 texId = getTextureID(type);

	auto shaderProgram = getShaderProgram();
	const GMString& uniform = getTextureUniformName(type);

	GMsize_t index = verifyIndicesContainer(d->textureIndices, shaderProgram);
	GMsize_t typeIndex = (GMsize_t)type;
	if (!d->textureIndices[index].Texture[typeIndex])
		d->textureIndices[index].Texture[typeIndex] = shaderProgram->getIndex(uniform + u_tex_texture);
	if (!d->textureIndices[index].Enabled[typeIndex])
		d->textureIndices[index].Enabled[typeIndex] = shaderProgram->getIndex(uniform + u_tex_enabled);
	shaderProgram->setInt(d->textureIndices[index].Texture[typeIndex], texId);
	shaderProgram->setInt(d->textureIndices[index].Enabled[typeIndex], 1);

	activateTextureTransform(model, type);
	return texId;
}

void GMGLTechnique::deactivateTexture(GMTextureType type)
{
	static const GMString u_tex_enabled = L"." + GM_VariablesDesc.TextureAttributes.Enabled;
	D(d);
	auto shaderProgram = getShaderProgram();
	GMsize_t index = verifyIndicesContainer(d->textureIndices, shaderProgram);
	const GMString& uniform = getTextureUniformName(type);
	GMsize_t typeIndex = (GMsize_t)type;
	if (!d->textureIndices[index].Enabled[typeIndex])
		d->textureIndices[index].Enabled[typeIndex] = shaderProgram->getIndex(uniform + u_tex_enabled);
	shaderProgram->setInt(d->textureIndices[index].Enabled[typeIndex], 0);
}

GMint32 GMGLTechnique::getTextureID(GMTextureType type)
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

bool GMGLTechnique::drawTexture(GMModel* model, GMTextureType type)
{
	D(d);
	if (d->engine->isNeedDiscardTexture(model, type))
		return false;

	// 按照贴图类型选择纹理动画序列
	GMTextureSampler& sampler = model->getShader().getTextureList().getTextureSampler(type);

	// 获取序列中的这一帧
	GMTextureAsset texture = getTexture(sampler);
	if (!texture.isEmpty())
	{
		// 激活动画序列
		GMint32 texId = activateTexture(model, (GMTextureType)type);
		texture.getTexture()->bindSampler(&sampler);
		texture.getTexture()->useTexture(texId);
		return true;
	}
	return false;
}

GMTextureAsset GMGLTechnique::getTexture(GMTextureSampler& frames)
{
	if (frames.getFrameCount() == 0)
		return GMAsset::invalidAsset();

	if (frames.getFrameCount() == 1)
		return frames.getFrameByIndex(0);

	// 如果frameCount > 1，说明是个动画，要根据Shader的间隔来选择合适的帧
	// TODO
	GMint32 elapsed = GM.getRunningStates().elapsedTime * 1000;

	return frames.getFrameByIndex((elapsed / frames.getAnimationMs()) % frames.getFrameCount());
}

void GMGLTechnique::updateCameraMatrices(IShaderProgram* shaderProgram)
{
	D(d);
	GMCamera& camera = d->engine->getCamera();
	if (s_shaderCaches.lastShaderProgram_camera != shaderProgram || camera.isDirty())
	{
		const GMMat4& viewMatrix = camera.getViewMatrix();
		const GMCameraLookAt& lookAt = camera.getLookAt();
		GMFloat4 viewPosition;
		lookAt.position.loadFloat4(viewPosition);

		GMGraphicEngine::getDefaultShaderVariablesDesc();
		shaderProgram->setVec4(VI(ViewPosition), viewPosition);
		shaderProgram->setMatrix4(VI(ViewMatrix), camera.getViewMatrix());
		shaderProgram->setMatrix4(VI(InverseViewMatrix), camera.getInverseViewMatrix());
		shaderProgram->setMatrix4(VI(ProjectionMatrix), camera.getProjectionMatrix());
		s_shaderCaches.lastShaderProgram_camera = shaderProgram;
		camera.cleanDirty();
	}
}

void GMGLTechnique::prepareScreenInfo(IShaderProgram* shaderProgram)
{
	D(d);
	static const GMString s_multisampling = GM_VariablesDesc.ScreenInfoAttributes.ScreenInfo + L"." + GM_VariablesDesc.ScreenInfoAttributes.Multisampling;
	static const GMString s_screenWidth = GM_VariablesDesc.ScreenInfoAttributes.ScreenInfo + L"." + GM_VariablesDesc.ScreenInfoAttributes.ScreenWidth;
	static const GMString s_screenHeight = GM_VariablesDesc.ScreenInfoAttributes.ScreenInfo + L"." + GM_VariablesDesc.ScreenInfoAttributes.ScreenHeight;
	if (s_shaderCaches.lastShaderProgram_screenInfo != shaderProgram) //或者窗口属性发生改变
	{
		const GMWindowStates& windowStates = d->context->getWindow()->getWindowStates();
		shaderProgram->setInt(VI_N(ScreenInfoAttributes.Multisampling, s_multisampling), windowStates.sampleCount > 1);
		shaderProgram->setInt(VI_N(ScreenInfoAttributes.ScreenWidth, s_screenWidth), windowStates.renderRect.width);
		shaderProgram->setInt(VI_N(ScreenInfoAttributes.ScreenHeight, s_screenHeight), windowStates.renderRect.height);
		s_shaderCaches.lastShaderProgram_screenInfo = shaderProgram;
	}
}

void GMGLTechnique::dirtyShadowMapAttributes()
{
	g_shadowDirty = true;
}

void GMGLTechnique::prepareShaderAttributes(GMModel* model)
{
	prepareBlend(model);
	prepareFrontFace(model);
	prepareCull(model);
	prepareDepth(model);
	prepareDebug(model);
}

void GMGLTechnique::prepareCull(GMModel* model)
{
	const GMShader& shader = model->getShader();
	if (shader.getCull() == GMS_Cull::Cull)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

void GMGLTechnique::prepareFrontFace(GMModel* model)
{
	const GMShader& shader = model->getShader();
	if (shader.getFrontFace() == GMS_FrontFace::Closewise)
		glFrontFace(GL_CW);
	else
		glFrontFace(GL_CCW);
}

void GMGLTechnique::prepareBlend(GMModel* model)
{
	D(d);
	bool blend = false;
	const GMShader& shader = model->getShader();
	const GMGlobalBlendStateDesc& globalBlendState = d->engine->getGlobalBlendState();
	if (globalBlendState.enabled)
	{
		blend = true;
		if (shader.getBlend())
		{
			blend = true;
			glEnable(GL_BLEND);
			GMGLUtility::blendFunc(
				shader.getBlendFactorSourceRGB(),
				shader.getBlendFactorDestRGB(),
				shader.getBlendOpRGB(),
				shader.getBlendFactorSourceAlpha(),
				shader.getBlendFactorDestAlpha(),
				shader.getBlendOpAlpha()
			);
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}
	else
	{
		if (shader.getBlend())
		{
			blend = true;
			glEnable(GL_BLEND);
			GMGLUtility::blendFunc(
				shader.getBlendFactorSourceRGB(),
				shader.getBlendFactorDestRGB(),
				shader.getBlendOpRGB(),
				shader.getBlendFactorSourceAlpha(),
				shader.getBlendFactorDestAlpha(),
				shader.getBlendOpAlpha()
			);
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}
}

void GMGLTechnique::prepareDepth(GMModel* model)
{
	const GMShader& shader = model->getShader();
	if (shader.getNoDepthTest())
		glDisable(GL_DEPTH_TEST); // glDepthMask(GL_FALSE);
	else
		glEnable(GL_DEPTH_TEST); // glDepthMask(GL_TRUE);
}

void GMGLTechnique::prepareDebug(GMModel* model)
{
	D(d);
	IShaderProgram* shaderProgram = getShaderProgram();
	GMint32 mode = d->debugConfig.get(gm::GMDebugConfigs::DrawPolygonNormalMode).toInt();
	shaderProgram->setInt(VI(Debug.Normal), mode);
}

void GMGLTechnique::prepareLights()
{
	D(d);
	d->engine->activateLights(this);
}

GMIlluminationModel GMGLTechnique::prepareIlluminationModel(GMModel* model)
{
	D(d);
	IShaderProgram* shaderProgram = getShaderProgram();
	GMShader& shader = model->getShader();
	GMIlluminationModel illuminationModel = shader.getIlluminationModel();
	shaderProgram->setInt(VI(IlluminationModel), (GMint32)illuminationModel);
	return illuminationModel;
}

void GMGLTechnique::updateBoneTransforms(IShaderProgram* shaderProgram, GMModel* model)
{
	D(d);
	static Vector<GMString> boneVarNames;
	if (boneVarNames.empty())
	{
		boneVarNames.resize(GMScene::MaxBoneCount);
		for (GMint32 i = 0; i < GMScene::MaxBoneCount; ++i)
		{
			boneVarNames[i] = (GMString(GM_VariablesDesc.Bones) + L"[" + GMString(i) + L"]");
		}
	}

	const auto& transforms = model->getBoneTransformations();
	for (GMsize_t i = 0; i < transforms.size(); ++i)
	{
		const auto& transform = transforms[i];
		shaderProgram->setMatrix4(getVariableIndex(shaderProgram, d->boneVariableIndices[i], boneVarNames[i]), transform);
	}
}

void GMGLTechnique::startDraw(GMModel* model)
{
	D(d);
	GLenum mode = (d->engine->isWireFrameMode(model)) ? GL_LINE_LOOP : getMode(model->getPrimitiveTopologyMode());
	if (model->getDrawMode() == GMModelDrawMode::Vertex)
		glDrawArrays(mode, 0, gm_sizet_to<GLsizei>(model->getVerticesCount()));
	else
		glDrawElements(mode, gm_sizet_to<GLsizei>(model->getVerticesCount()), GL_UNSIGNED_INT, 0);
}

//////////////////////////////////////////////////////////////////////////
void GMGLTechnique_3D::beginModel(GMModel* model, const GMGameObject* parent)
{
	Base::beginModel(model, parent);

	D(d);
	D_BASE(db, GMGLTechnique);
	auto shaderProgram = getShaderProgram();
	updateCameraMatrices(shaderProgram);

	if (parent)
	{
		shaderProgram->setMatrix4(VI_B(ModelMatrix), parent->getTransform());
		shaderProgram->setMatrix4(VI_B(InverseTransposeModelMatrix), InverseTranspose(parent->getTransform()));
	}
	else
	{
		shaderProgram->setMatrix4(VI_B(ModelMatrix), Identity<GMMat4>());
		shaderProgram->setMatrix4(VI_B(InverseTransposeModelMatrix), Identity<GMMat4>());
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
		prepareLights();
	}

	const GMShadowSourceDesc& shadowSourceDesc = db->engine->getShadowSourceDesc();
	if (shadowSourceDesc.type != GMShadowSourceDesc::NoShadow)
	{
		GMGLShadowFramebuffers* shadowFramebuffers = gm_cast<GMGLShadowFramebuffers*>(db->engine->getShadowMapFramebuffers());
		shadowFramebuffers->getShadowMapTexture().getTexture()->useTexture(0);
		applyShadow(this, &shadowSourceDesc, shaderProgram, shadowFramebuffers, true);
	}
	else
	{
		applyShadow(this, nullptr, shaderProgram, nullptr, false);
	}

	db->gammaHelper.setGamma(this, db->engine, shaderProgram);
}

void GMGLTechnique_3D::beforeDraw(GMModel* model)
{
	D(d);
	D_BASE(db, GMGLTechnique);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// 材质
	prepareMaterial(model->getShader());

	// 应用Shader
	prepareShaderAttributes(model);
	
	// 设置光照模型
	GMIlluminationModel illuminationModel = prepareIlluminationModel(model);

	// 纹理
	prepareTextures(model, illuminationModel);

	// 调试绘制
	drawDebug();
}

void GMGLTechnique_3D::afterDraw(GMModel* model)
{
	D(d);
	GM_FOREACH_ENUM_CLASS(type, GMTextureType::Ambient, GMTextureType::EndOfCommonTexture)
	{
		deactivateTexture((GMTextureType)type);
	}
}

IShaderProgram* GMGLTechnique_3D::getShaderProgram()
{
	D_BASE(d, Base);
	GMGeometryPassingState s = d->engine->getGBuffer()->getGeometryPassingState();
	if (s == GMGeometryPassingState::PassingGeometry)
		return d->engine->getShaderProgram(GMShaderProgramType::DeferredGeometryPassShaderProgram);

	return d->engine->getShaderProgram(GMShaderProgramType::ForwardShaderProgram);
}

void GMGLTechnique_3D::prepareMaterial(const GMShader& shader)
{
	D(d);
	D_BASE(db, Base);
	auto shaderProgram = getShaderProgram();
	static const GMString GMSHADER_MATERIAL_KA = GM_VariablesDesc.MaterialName + L"." + GM_VariablesDesc.MaterialAttributes.Ka;
	static const GMString GMSHADER_MATERIAL_KD = GM_VariablesDesc.MaterialName + L"." + GM_VariablesDesc.MaterialAttributes.Kd;
	static const GMString GMSHADER_MATERIAL_KS = GM_VariablesDesc.MaterialName + L"." + GM_VariablesDesc.MaterialAttributes.Ks;
	static const GMString GMSHADER_MATERIAL_SHININESS = GM_VariablesDesc.MaterialName + L"." + GM_VariablesDesc.MaterialAttributes.Shininess;
	static const GMString GMSHADER_MATERIAL_REFRACTIVITY = GM_VariablesDesc.MaterialName + L"." + GM_VariablesDesc.MaterialAttributes.Refreactivity;
	static const GMString GMSHADER_MATERIAL_F0 = GM_VariablesDesc.MaterialName + L"." + GM_VariablesDesc.MaterialAttributes.F0;

	const GMMaterial& material = shader.getMaterial();
	shaderProgram->setVec3(VI_NB(MaterialAttributes.Ka, GMSHADER_MATERIAL_KA), ValuePointer(material.getAmbient()));
	shaderProgram->setVec3(VI_NB(MaterialAttributes.Kd, GMSHADER_MATERIAL_KD), ValuePointer(material.getDiffuse()));
	shaderProgram->setVec3(VI_NB(MaterialAttributes.Ks, GMSHADER_MATERIAL_KS), ValuePointer(material.getSpecular()));
	shaderProgram->setFloat(VI_NB(MaterialAttributes.Shininess, GMSHADER_MATERIAL_SHININESS), material.getShininess());
	shaderProgram->setFloat(VI_NB(MaterialAttributes.Refreactivity, GMSHADER_MATERIAL_REFRACTIVITY), material.getRefractivity());
	shaderProgram->setVec3(VI_NB(MaterialAttributes.F0, GMSHADER_MATERIAL_F0), ValuePointer(material.getF0()));
}

void GMGLTechnique_3D::prepareTextures(GMModel* model, GMIlluminationModel illuminationModel)
{
	GM_FOREACH_ENUM_CLASS(type, GMTextureType::Ambient, GMTextureType::EndOfCommonTexture)
	{
		if (!drawTexture(model, (GMTextureType)type))
		{
			if (illuminationModel == GMIlluminationModel::Phong && (
				type == GMTextureType::Ambient ||
				type == GMTextureType::Diffuse ||
				type == GMTextureType::Specular ||
				type == GMTextureType::Lightmap
				))
			{
				GMint32 texId = activateTexture(nullptr, (GMTextureType)type);
				getWhiteTexture().getTexture()->useTexture(texId);
			}
		}
	}
}

void GMGLTechnique_3D::drawDebug()
{
	D(d);
	D_BASE(db, Base);
	static const GMString s_GMSHADER_DEBUG_DRAW_NORMAL = GMSHADER_DEBUG_DRAW_NORMAL;
	auto shaderProgram = getShaderProgram();
	shaderProgram->setInt(getVariableIndex(shaderProgram, d->drawDebugNormalIndex, s_GMSHADER_DEBUG_DRAW_NORMAL), db->debugConfig.get(GMDebugConfigs::DrawPolygonNormalMode).toInt());
}

GMTextureAsset GMGLTechnique_3D::getWhiteTexture()
{
	D(d);
	D_BASE(db, Base);
	if (d->whiteTexture.isEmpty())
		d->whiteTexture = createWhiteTexture(db->context);
	return d->whiteTexture;
}

//////////////////////////////////////////////////////////////////////////
void GMGLTechnique_2D::beforeDraw(GMModel* model)
{
	D(d);
	// 应用Shader
	prepareShaderAttributes(model);

	// 纹理
	prepareTextures(model);
}

void GMGLTechnique_2D::prepareTextures(GMModel* model)
{
	// 只选择环境光纹理
	GMTextureSampler& sampler = model->getShader().getTextureList().getTextureSampler(GMTextureType::Ambient);

	// 获取序列中的这一帧
	GMTextureAsset texture = getTexture(sampler);
	if (!texture.isEmpty())
	{
		// 激活动画序列
		GMint32 texId = activateTexture(model, GMTextureType::Ambient);
		texture.getTexture()->bindSampler(&sampler);
		texture.getTexture()->useTexture(texId);
	}
}

//////////////////////////////////////////////////////////////////////////
void GMGLTechnique_CubeMap::beginModel(GMModel* model, const GMGameObject* parent)
{
	D_BASE(db, Base);
	Base::beginModel(model, parent);
	IShaderProgram* shaderProgram = getShaderProgram();
	updateCameraMatrices(shaderProgram);
	shaderProgram->setMatrix4(VI_B(ModelMatrix), GMMat4(Inhomogeneous(parent->getTransform())));
}

void GMGLTechnique_CubeMap::beforeDraw(GMModel* model)
{
	D(d);
	D_BASE(db, Base);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	prepareTextures(model);
}

void GMGLTechnique_CubeMap::afterDraw(GMModel* model)
{
}

void GMGLTechnique_CubeMap::prepareTextures(GMModel* model)
{
	D_BASE(db, Base);
	GMTextureList& texture = model->getShader().getTextureList();
	GMTextureSampler& sampler = texture.getTextureSampler(GMTextureType::CubeMap);
	GMTextureAsset glTex = sampler.getFrameByIndex(0);
	if (!glTex.isEmpty())
	{
		IShaderProgram* shaderProgram = getShaderProgram();
		GM_ASSERT(model->getType() == GMModelType::CubeMap);
		shaderProgram->setInterfaceInstance(GMGLShaderProgram::techniqueName(), getTechnique(model->getType()), GMShaderType::Vertex);
		shaderProgram->setInterfaceInstance(GMGLShaderProgram::techniqueName(), getTechnique(model->getType()), GMShaderType::Pixel);
		shaderProgram->setInt(VI_B(CubeMapTextureName), GMTextureRegisterQuery<GMTextureType::CubeMap>::Value);
		glTex.getTexture()->bindSampler(&sampler);
		glTex.getTexture()->useTexture(GMTextureRegisterQuery<GMTextureType::CubeMap>::Value);
		db->engine->setCubeMap(glTex);
	}
}
//////////////////////////////////////////////////////////////////////////
void GMGLTechnique_Filter::beforeDraw(GMModel* model)
{
	prepareShaderAttributes(model);
	prepareTextures(model);
}

void GMGLTechnique_Filter::afterDraw(GMModel* model)
{
}

void GMGLTechnique_Filter::beginModel(GMModel* model, const GMGameObject* parent)
{
	Base::beginModel(model, parent);

	D(d);
	D_BASE(db, Base);
	IShaderProgram* shaderProgram = getShaderProgram();
	if (d->state.HDR != db->engine->needHDR() || d->state.toneMapping != db->engine->getToneMapping())
	{
		d->state.HDR = db->engine->needHDR();
		d->state.toneMapping = db->engine->getToneMapping();
		if (d->state.HDR)
		{
			db->gammaHelper.setGamma(this, db->engine, shaderProgram);
			setHDR(shaderProgram);
		}
		else
		{
			shaderProgram->setBool(VI_B(HDR.HDR), false);
		}
	}
}

void GMGLTechnique_Filter::setHDR(IShaderProgram* shaderProgram)
{
	D(d);
	D_BASE(db, Base);
	shaderProgram->setBool(VI_B(HDR.HDR), true);
	shaderProgram->setInt(VI_B(HDR.ToneMapping), d->state.toneMapping);
}

IShaderProgram* GMGLTechnique_Filter::getShaderProgram()
{
	D_BASE(db, GMGLTechnique);
	return db->engine->getShaderProgram(GMShaderProgramType::FilterShaderProgram);
}

void GMGLTechnique_Filter::prepareTextures(GMModel* model)
{
	GMTextureSampler& sampler = model->getShader().getTextureList().getTextureSampler(GMTextureType::Ambient);
	GMTextureAsset texture = getTexture(sampler);
	GM_ASSERT(!texture.isEmpty());
	GMint32 texId = activateTexture(model, GMTextureType::Ambient);
	texture.getTexture()->bindSampler(&sampler);
	texture.getTexture()->useTexture(texId);
}

GMint32 GMGLTechnique_Filter::activateTexture(GMModel* model, GMTextureType type)
{
	D(d);
	D_BASE(db, Base);
	GMint32 texId = getTextureID(type);
	IShaderProgram* shaderProgram = getShaderProgram();
	shaderProgram->setInt(getVariableIndex(shaderProgram, d->framebufferIndex, GMSHADER_FRAMEBUFFER), texId);

	bool b = shaderProgram->setInterfaceInstance(GM_VariablesDesc.FilterAttributes.Filter, GM_VariablesDesc.FilterAttributes.Types[db->engine->getCurrentFilterMode()], GMShaderType::Pixel);
	GM_ASSERT(b);
	return texId;
}

IShaderProgram* GMGLTechnique_LightPass::getShaderProgram()
{
	D_BASE(db, GMGLTechnique);
	return db->engine->getShaderProgram(GMShaderProgramType::DeferredLightPassShaderProgram);
}

void GMGLTechnique_LightPass::beginModel(GMModel* model, const GMGameObject* parent)
{
	GMGLTechnique::beginModel(model, parent);

	D_BASE(d, GMGLTechnique);
	IShaderProgram* shaderProgram = getShaderProgram();
	updateCameraMatrices(shaderProgram);

	const GMShadowSourceDesc& shadowSourceDesc = d->engine->getShadowSourceDesc();
	if (shadowSourceDesc.type != GMShadowSourceDesc::NoShadow)
	{
		GMGLShadowFramebuffers* shadowFramebuffers = gm_cast<GMGLShadowFramebuffers*>(d->engine->getShadowMapFramebuffers());
		shadowFramebuffers->getShadowMapTexture().getTexture()->useTexture(0);
		applyShadow(this, &shadowSourceDesc, shaderProgram, shadowFramebuffers, true);
	}
	else
	{
		applyShadow(this, nullptr, shaderProgram, nullptr, false);
	}

	d->gammaHelper.setGamma(this, d->engine, shaderProgram);
}

void GMGLTechnique_LightPass::afterDraw(GMModel* model)
{
}

void GMGLTechnique_LightPass::beforeDraw(GMModel* model)
{
	prepareLights();
	prepareTextures(model);
}

void GMGLTechnique_LightPass::prepareLights()
{
	D_BASE(d, GMGLTechnique);
	d->engine->activateLights(this);
}

void GMGLTechnique_LightPass::prepareTextures(GMModel* model)
{
	D(d);
	D_BASE(db, GMGLTechnique);
	IShaderProgram* shaderProgram = getShaderProgram();
	IGBuffer* gBuffer = db->engine->getGBuffer();
	IFramebuffers* gBufferFramebuffers = gBuffer->getGeometryFramebuffers();
	GMsize_t cnt = gBufferFramebuffers->count();

	GMsize_t shaderIdx = verifyIndicesContainer(d->gbufferIndices, shaderProgram);
	if (d->gbufferIndices[shaderIdx].size() <= cnt)
		d->gbufferIndices[shaderIdx].resize(cnt + 1);

	for (GMsize_t i = 0; i < cnt; ++i)
	{
		GMTextureAsset texture;
		gBufferFramebuffers->getFramebuffer(i)->getTexture(texture);
		const GMsize_t textureIndex = (GMTextureRegisterQuery<GMTextureType::GeometryPasses>::Value + i);
		shaderProgram->setInt( getVariableIndex(shaderProgram, d->gbufferIndices[shaderIdx][i], GMGLGBuffer::GBufferGeometryUniformNames()[i]), gm_sizet_to_uint(textureIndex));
		texture.getTexture()->useTexture((GMuint32)textureIndex);
	}

	GMTextureAsset cubeMap = db->engine->getCubeMap();
	if (!cubeMap.isEmpty())
	{
		const GMsize_t id = GMTextureRegisterQuery<GMTextureType::GeometryPasses>::Value + 1 + cnt;
		shaderProgram->setInt(VI_B(CubeMapTextureName), gm_sizet_to_uint(id));
		cubeMap.getTexture()->useTexture((GMuint32)id);
	}
}

void GMGLTechnique_3D_Shadow::beginModel(GMModel* model, const GMGameObject* parent)
{
	GMGLTechnique_3D::beginModel(model, parent);

	D_BASE(d, Base);
	IShaderProgram* shaderProgram = getShaderProgram();

	if (parent)
		shaderProgram->setMatrix4(VI(ModelMatrix), parent->getTransform());
	else
		shaderProgram->setMatrix4(VI(ModelMatrix), Identity<GMMat4>());

	GMGLShadowFramebuffers* shadowFramebuffers = gm_cast<GMGLShadowFramebuffers*>(d->engine->getShadowMapFramebuffers());
	applyShadow(this, &d->engine->getShadowSourceDesc(), shaderProgram, shadowFramebuffers, true);

	static const GMString& s_shadow = "GM_Shadow";
	bool b = shaderProgram->setInterfaceInstance(
		GMGLShaderProgram::techniqueName(),
		s_shadow,
		GMShaderType::Vertex);
	GM_ASSERT(b);
	b = shaderProgram->setInterfaceInstance(
		GMGLShaderProgram::techniqueName(),
		s_shadow,
		GMShaderType::Pixel);
	GM_ASSERT(b);
}

IShaderProgram* GMGLTechnique_Custom::getShaderProgram()
{
	D_BASE(d, GMGLTechnique);
	GMRenderTechniqueManager* rtm = d->engine->getRenderTechniqueManager();
	IShaderProgram* shaderProgram = rtm->getShaderProgram(d->techContext.currentModel->getTechniqueId());
	GM_ASSERT(shaderProgram);
	return shaderProgram;
}