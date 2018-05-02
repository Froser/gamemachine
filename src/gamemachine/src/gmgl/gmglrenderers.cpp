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
	d->engine = gm_cast<GMGLGraphicEngine*>(GM.getGraphicEngine());
	d->debugConfig = GM.getConfigs().getConfig(GMConfigs::Debug).asDebugConfig();
}

void GMGLRenderer::draw(GMModel* model)
{
	D(d);
	applyStencil(*d->engine);
	prepareScreenInfo(getShaderProgram());
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

	auto shaderProgram = getShaderProgram();
	const char* uniform = getTextureUniformName(desc, type, index);

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

	auto shaderProgram = getShaderProgram();
	const char* uniform = getTextureUniformName(desc, type, index);
	static char u_texture[GMGL_MAX_UNIFORM_NAME_LEN], u_enabled[GMGL_MAX_UNIFORM_NAME_LEN];
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

	auto shaderProgram = getShaderProgram();
	GMint idx = (GMint)type;
	const char* uniform = getTextureUniformName(desc, type, index);
	static char u[GMGL_MAX_UNIFORM_NAME_LEN];
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

//////////////////////////////////////////////////////////////////////////
void GMGLRenderer_3D::beginModel(GMModel* model, const GMGameObject* parent)
{
	D(d);
	D_BASE(db, GMGLRenderer);
	auto shaderProgram = getShaderProgram();
	shaderProgram->useProgram();
	updateCameraMatrices(shaderProgram);

	auto desc = getVariablesDesc();
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

	GMTexture& texture = model->getShader().getTexture();
	GMTextureFrames& frames = texture.getTextureFrames(GMTextureType::CubeMap, 0);
	ITexture* glTex = frames.getFrameByIndex(0);
	if (glTex)
	{
		IShaderProgram* shaderProgram = getShaderProgram();
		auto desc = getVariablesDesc();
		GM_ASSERT(model->getType() == GMModelType::CubeMap);
		shaderProgram->setInterfaceInstance(GMGLShaderProgram::techniqueName(), getTechnique(model->getType()), GMShaderType::Vertex);
		shaderProgram->setInterfaceInstance(GMGLShaderProgram::techniqueName(), getTechnique(model->getType()), GMShaderType::Pixel);
		shaderProgram->setInt(desc->CubeMapTextureName, GMTextureRegisterQuery<GMTextureType::CubeMap>::Value);
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
	IShaderProgram* shaderProgram = getShaderProgram();
	GM_ASSERT(shaderProgram);
	shaderProgram->useProgram();
}

void GMGLRenderer_Filter::endModel()
{
}

IShaderProgram* GMGLRenderer_Filter::getShaderProgram()
{
	D_BASE(db, GMGLRenderer);
	return db->engine->getShaderProgram(GMShaderProgramType::FilterShaderProgram);
}

GMint GMGLRenderer_Filter::activateTexture(GMModel* model, GMTextureType type, GMint index)
{
	D(d);
	GMint texId = getTextureID(type, index);
	IShaderProgram* shaderProgram = getShaderProgram();
	shaderProgram->setInt(GMSHADER_FRAMEBUFFER, texId);

	const GMShaderVariablesDesc* desc = getVariablesDesc();
	bool b = shaderProgram->setInterfaceInstance(desc->FilterAttributes.Filter, desc->FilterAttributes.Types[d->engine->getCurrentFilterMode()], GMShaderType::Pixel);
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
	GMuint cnt = gBufferFramebuffers->count();
	for (GMuint i = 0; i < cnt; ++i)
	{
		ITexture* texture = gBufferFramebuffers->getFramebuffer(i)->getTexture();
		shaderProgram->setInt(GMGLGBuffer::GBufferGeometryUniformNames()[i].c_str(), i);
		texture->useTexture(nullptr, i);
	}

	ITexture* cubeMap = d->engine->getCubeMap();
	if (cubeMap)
	{
		const GMuint id = cnt + 1;
		shaderProgram->setInt(getVariablesDesc()->CubeMapTextureName, id);
		cubeMap->useTexture(nullptr, id);
	}
}
