#include "stdafx.h"
#include "gmdx11renderers.h"
#include "gmdata/gmmodel.h"
#include <gmgameobject.h>
#include <gmdx11helper.h>
#include "gmdx11modeldataproxy.h"
#include "gmdx11texture.h"
#include "gmdx11graphic_engine.h"
#include "gmdx11gbuffer.h"
#include "gmdx11framebuffer.h"

#ifdef max
#undef max
#endif

#define GMSHADER_SEMANTIC_NAME_POSITION "POSITION"
#define GMSHADER_SEMANTIC_NAME_NORMAL "NORMAL"
#define GMSHADER_SEMANTIC_NAME_TEXCOORD "TEXCOORD"
#define GMSHADER_SEMANTIC_NAME_TANGENT "NORMAL"
#define GMSHADER_SEMANTIC_NAME_BITANGENT "NORMAL"
#define GMSHADER_SEMANTIC_NAME_LIGHTMAP "TEXCOORD"
#define GMSHADER_SEMANTIC_NAME_COLOR "COLOR"
#define FLOAT_OFFSET(i) (sizeof(gm::GMfloat) * i)
#define CHECK_VAR(var) if (!var->IsValid()) { GM_ASSERT(false); return; }

namespace
{
	D3D11_INPUT_ELEMENT_DESC GMSHADER_ElementDescriptions[] =
	{
		{ GMSHADER_SEMANTIC_NAME_POSITION, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, FLOAT_OFFSET(0), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 3

		{ GMSHADER_SEMANTIC_NAME_NORMAL, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, FLOAT_OFFSET(3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 3

		{ GMSHADER_SEMANTIC_NAME_TEXCOORD, 0, DXGI_FORMAT_R32G32_FLOAT, 0, FLOAT_OFFSET(6), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 2

		{ GMSHADER_SEMANTIC_NAME_TANGENT, 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, FLOAT_OFFSET(8), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 3

		{ GMSHADER_SEMANTIC_NAME_BITANGENT, 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, FLOAT_OFFSET(11), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 3

		{ GMSHADER_SEMANTIC_NAME_LIGHTMAP, 1, DXGI_FORMAT_R32G32_FLOAT, 0, FLOAT_OFFSET(14), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 2

		{ GMSHADER_SEMANTIC_NAME_COLOR, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, FLOAT_OFFSET(16), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 4
	};

	inline D3D_PRIMITIVE_TOPOLOGY getMode(GMTopologyMode mode)
	{
		switch (mode)
		{
		case GMTopologyMode::TriangleStrip:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		case GMTopologyMode::Triangles:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case GMTopologyMode::Lines:
			return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
		default:
			GM_ASSERT(false);
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
	}

	inline D3D11_RASTERIZER_DESC getRasterizerDesc(
		GMS_FrontFace frontFace,
		GMS_Cull cull,
		bool multisampleEnable,
		bool antialiasedLineEnable
	)
	{
		D3D11_RASTERIZER_DESC desc = {
			D3D11_FILL_SOLID,
			cull == GMS_Cull::CULL ? D3D11_CULL_BACK : D3D11_CULL_NONE,
			frontFace == GMS_FrontFace::CLOCKWISE ? FALSE : TRUE,
			0,
			0.f,
			0.f,
			TRUE,
			FALSE,
			multisampleEnable ? TRUE : FALSE,
			antialiasedLineEnable ? TRUE : FALSE
		};
		return desc;
	}

	inline D3D11_BLEND toDx11Blend(GMS_BlendFunc blendFunc)
	{
		switch (blendFunc)
		{
		case (GMS_BlendFunc::ZERO):
			return D3D11_BLEND_ZERO;
		case (GMS_BlendFunc::ONE):
			return D3D11_BLEND_ONE;
		case (GMS_BlendFunc::SRC_COLOR):
			return D3D11_BLEND_SRC_COLOR;
		case (GMS_BlendFunc::DST_COLOR):
			return D3D11_BLEND_DEST_COLOR;
		case (GMS_BlendFunc::SRC_ALPHA):
			return D3D11_BLEND_SRC_ALPHA;
		case (GMS_BlendFunc::DST_ALPHA):
			return D3D11_BLEND_DEST_ALPHA;
		case (GMS_BlendFunc::ONE_MINUS_SRC_ALPHA):
			return D3D11_BLEND_INV_SRC_ALPHA;
		case (GMS_BlendFunc::ONE_MINUS_DST_ALPHA):
			return D3D11_BLEND_INV_DEST_ALPHA;
		case (GMS_BlendFunc::ONE_MINUS_DST_COLOR):
			return D3D11_BLEND_INV_DEST_COLOR;
		default:
			GM_ASSERT(false);
			return D3D11_BLEND_ONE;
		}
	}

	inline D3D11_BLEND_DESC getBlendDesc(
		bool enabled,
		GMS_BlendFunc source,
		GMS_BlendFunc dest
	)
	{
		D3D11_BLEND_DESC desc = { 0 };
		for (GMuint i = 0; i < 8; ++i)
		{
			desc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}

		D3D11_RENDER_TARGET_BLEND_DESC& renderTarget = desc.RenderTarget[0];
		if (!enabled)
		{
			renderTarget.BlendEnable = FALSE;
			return desc;
		}
		else
		{
			renderTarget.BlendEnable = TRUE;
			renderTarget.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			renderTarget.SrcBlend = toDx11Blend(source);
			renderTarget.SrcBlendAlpha = D3D11_BLEND_ONE;
			renderTarget.DestBlend = toDx11Blend(dest);
			renderTarget.DestBlendAlpha = D3D11_BLEND_ZERO;
			renderTarget.BlendOp = D3D11_BLEND_OP_ADD; //目前不提供其他Blend操作
			renderTarget.BlendOpAlpha = D3D11_BLEND_OP_ADD; //目前不提供其他Blend操作
			return desc;
		}
	}

	inline D3D11_DEPTH_STENCIL_DESC getDepthStencilDesc(
		bool depthEnabled,
		const GMStencilOptions& stencilOptions
	)
	{
		D3D11_DEPTH_STENCIL_DESC desc = { 0 };
		desc.DepthEnable = depthEnabled ? TRUE : FALSE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS;

		desc.StencilEnable = TRUE;
		desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		desc.StencilWriteMask = stencilOptions.writeMask;

		desc.FrontFace.StencilFailOp = desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilDepthFailOp = desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilPassOp = desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		desc.FrontFace.StencilFunc = desc.BackFace.StencilFunc = 
			(stencilOptions.compareOp == GMStencilOptions::Equal ? D3D11_COMPARISON_EQUAL :
			(stencilOptions.compareOp == GMStencilOptions::NotEqual ? D3D11_COMPARISON_NOT_EQUAL : D3D11_COMPARISON_ALWAYS)
		);
		return desc;
	}

	struct GMDx11RasterizerStates : public GMSingleton<GMDx11RasterizerStates>
	{
		enum
		{
			Size_Cull = 2,
			Size_FrontFace = 2,
		};

		//TODO 先不考虑FillMode
	public:
		GMDx11RasterizerStates::GMDx11RasterizerStates()
		{
			engine = gm_cast<GMDx11GraphicEngine*>(GM.getGraphicEngine());
		}

		~GMDx11RasterizerStates()
		{
			for (GMint i = 0; i < Size_Cull; ++i)
			{
				for (GMint j = 0; j < Size_FrontFace; ++j)
				{
					if (!states[i][j])
						continue;
					states[i][j]->Release();
				}
			}
		}

	public:
		ID3D11RasterizerState* getRasterStates(GMS_FrontFace frontFace, GMS_Cull cullMode)
		{
			bool multisampleEnable = GM.getGameMachineRunningStates().sampleCount > 1;
			if (!states[(GMuint)cullMode][(GMuint)frontFace])
			{
				D3D11_RASTERIZER_DESC desc = getRasterizerDesc(frontFace, cullMode, multisampleEnable, multisampleEnable);
				createRasterizerState(desc, &states[(GMuint)cullMode][(GMuint)frontFace]);
			}

			GM_ASSERT(states[(GMuint)cullMode][(GMuint)frontFace]);
			return states[(GMuint)cullMode][(GMuint)frontFace];
		}

	private:
		bool createRasterizerState(const D3D11_RASTERIZER_DESC& desc, ID3D11RasterizerState** out)
		{
			GM_DX_HR(engine->getDevice()->CreateRasterizerState(&desc, out));
			return !!(*out);
		}

	private:
		GMDx11GraphicEngine* engine = nullptr;
		ID3D11RasterizerState* states[Size_Cull][Size_FrontFace] = { 0 };
	};

	struct GMDx11BlendStates : public GMSingleton<GMDx11BlendStates>
	{
	public:
		GMDx11BlendStates::GMDx11BlendStates()
		{
			engine = gm_cast<GMDx11GraphicEngine*>(GM.getGraphicEngine());
		}

		~GMDx11BlendStates()
		{
			for (GMint b = 0; b < 2; ++b)
			{
				for (GMint i = 0; i < (GMuint)GMS_BlendFunc::MAX_OF_BLEND_FUNC; ++i)
				{
					for (GMint j = 0; j < (GMuint)GMS_BlendFunc::MAX_OF_BLEND_FUNC; ++j)
					{
						if (states[b][i][j])
							states[b][i][j]->Release();
					}
				}
			}
		}

	public:
		ID3D11BlendState* getBlendState(bool enable, GMS_BlendFunc src, GMS_BlendFunc dest)
		{
			ID3D11BlendState*& state = states[enable ? 1 : 0][(GMuint)src][(GMuint)dest];
			if (!state)
			{
				D3D11_BLEND_DESC desc = getBlendDesc(enable, src, dest);
				createBlendState(desc, &state);
			}

			GM_ASSERT(state);
			return state;
		}

		ID3D11BlendState* getDisabledBlendState()
		{
			return getBlendState(false, GMS_BlendFunc::ONE, GMS_BlendFunc::ONE);
		}

	private:
		bool createBlendState(const D3D11_BLEND_DESC& desc, ID3D11BlendState** out)
		{
			GM_DX_HR(engine->getDevice()->CreateBlendState(&desc, out));
			return !!(*out);
		}

	private:
		GMDx11GraphicEngine* engine = nullptr;
		ID3D11BlendState* states[2][(GMuint)GMS_BlendFunc::MAX_OF_BLEND_FUNC][(GMuint)GMS_BlendFunc::MAX_OF_BLEND_FUNC] = { 0 };
	};

	struct GMDx11DepthStencilStates : public GMSingleton<GMDx11DepthStencilStates>
	{
	public:
		GMDx11DepthStencilStates::GMDx11DepthStencilStates()
		{
			engine = gm_cast<GMDx11GraphicEngine*>(GM.getGraphicEngine());
		}

		~GMDx11DepthStencilStates()
		{
			for (GMint b1 = 0; b1 < 2; ++b1)
			{
				for (GMint b2 = 0; b2 < 2; ++b2)
				{
					for (GMint b3 = 0; b3 < 3; ++b3)
					{
						if (states[b1][b2][b3])
							states[b1][b2][b3]->Release();
					}
				}
			}
		}

	public:
		ID3D11DepthStencilState* getDepthStencilState(bool depthEnabled, const GMStencilOptions& stencilOptions)
		{
			ID3D11DepthStencilState*& state = states[depthEnabled ? 1 : 0][stencilOptions.writeMask == GMStencilOptions::Ox00 ? 1 : 0][stencilOptions.compareOp];
			if (!state)
			{
				D3D11_DEPTH_STENCIL_DESC desc = getDepthStencilDesc(depthEnabled, stencilOptions);
				createDepthStencilState(desc, &state);
			}

			GM_ASSERT(state);
			return state;
		}

	private:
		bool createDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& desc, ID3D11DepthStencilState** out)
		{
			GM_DX_HR(engine->getDevice()->CreateDepthStencilState(&desc, out));
			return !!(*out);
		}

	private:
		GMDx11GraphicEngine* engine = nullptr;
		ID3D11DepthStencilState* states[2][2][3] = { 0 };
	};
}

#define EFFECT_VARIABLE(funcName, name) \
	ID3DX11EffectVariable* funcName() {												\
		static ID3DX11EffectVariable* s_ptr = nullptr;								\
		if (!s_ptr) {																\
			s_ptr = m_effect->GetVariableByName(name);								\
			GM_ASSERT(s_ptr->IsValid()); }											\
		return s_ptr;																\
	}

#define EFFECT_VARIABLE_AS(funcName, name, retType, to) \
	retType* funcName() {															\
		static retType* s_ptr = nullptr;											\
		if (!s_ptr) {																\
			s_ptr = m_effect->GetVariableByName(name)->to();						\
			GM_ASSERT(s_ptr->IsValid()); }											\
		return s_ptr;																\
	}

#define EFFECT_VARIABLE_AS_SHADER_RESOURCE(funcName, name) \
	EFFECT_VARIABLE_AS(funcName, name, ID3DX11EffectShaderResourceVariable, AsShaderResource)

#define EFFECT_VARIABLE_AS_SCALAR(funcName, name) \
	EFFECT_VARIABLE_AS(funcName, name, ID3DX11EffectScalarVariable, AsScalar)

#define EFFECT_MEMBER_AS(funcName, effect, name, retType, to) \
	retType* funcName() {															\
		static retType* s_ptr = nullptr;											\
		if (!s_ptr) {																\
			s_ptr = effect->GetMemberByName(name)->to();							\
			GM_ASSERT(s_ptr->IsValid()); }											\
		return s_ptr;																\
	}

#define EFFECT_MEMBER_AS_SCALAR(funcName, effect, name) \
	EFFECT_MEMBER_AS(funcName, effect, name, ID3DX11EffectScalarVariable, AsScalar)

#define EFFECT_MEMBER_AS_MATRIX(funcName, effect, name) \
	EFFECT_MEMBER_AS(funcName, effect, name, ID3DX11EffectMatrixVariable, AsMatrix)

#define EFFECT_MEMBER_AS_VECTOR(funcName, effect, name) \
	EFFECT_MEMBER_AS(funcName, effect, name, ID3DX11EffectVectorVariable, AsVector)

class GMDx11EffectVariableBank
{
public:
	void init(ID3DX11Effect* effect, const GMShaderVariablesDesc* desc)
	{
		m_effect = effect;
		m_desc = desc;
	}

	// Shadow
	EFFECT_VARIABLE(ShadowInfo, m_desc->ShadowInfo.ShadowInfo)
	EFFECT_MEMBER_AS_SCALAR(HasShadow, ShadowInfo(), m_desc->ShadowInfo.HasShadow)
	EFFECT_MEMBER_AS_MATRIX(ShadowMatrix, ShadowInfo(), m_desc->ShadowInfo.ShadowMatrix)
	EFFECT_MEMBER_AS_VECTOR(ShadowPosition, ShadowInfo(), m_desc->ShadowInfo.Position)
	EFFECT_MEMBER_AS_SCALAR(ShadowMapWidth, ShadowInfo(), m_desc->ShadowInfo.ShadowMapWidth)
	EFFECT_MEMBER_AS_SCALAR(ShadowMapHeight, ShadowInfo(), m_desc->ShadowInfo.ShadowMapHeight)
	EFFECT_MEMBER_AS_SCALAR(ShadowBiasMin, ShadowInfo(), m_desc->ShadowInfo.BiasMin)
	EFFECT_MEMBER_AS_SCALAR(ShadowBiasMax, ShadowInfo(), m_desc->ShadowInfo.BiasMax)
	EFFECT_VARIABLE_AS_SHADER_RESOURCE(ShadowMap, m_desc->ShadowInfo.ShadowMap)
	EFFECT_VARIABLE_AS_SHADER_RESOURCE(ShadowMapMSAA, m_desc->ShadowInfo.ShadowMapMSAA)

	// ScreenInfo
	EFFECT_VARIABLE(ScreenInfo, m_desc->ScreenInfoAttributes.ScreenInfo)
	EFFECT_MEMBER_AS_SCALAR(ScreenWidth, ScreenInfo(), m_desc->ScreenInfoAttributes.ScreenWidth)
	EFFECT_MEMBER_AS_SCALAR(ScreenHeight, ScreenInfo(), m_desc->ScreenInfoAttributes.ScreenHeight)
	EFFECT_MEMBER_AS_SCALAR(ScreenMultiSampling, ScreenInfo(), m_desc->ScreenInfoAttributes.Multisampling)

	// Material
	EFFECT_VARIABLE(Material, m_desc->MaterialName)
	EFFECT_MEMBER_AS_VECTOR(Ka, Material(), m_desc->MaterialAttributes.Ka)
	EFFECT_MEMBER_AS_VECTOR(Kd, Material(), m_desc->MaterialAttributes.Kd)
	EFFECT_MEMBER_AS_VECTOR(Ks, Material(), m_desc->MaterialAttributes.Ks)
	EFFECT_MEMBER_AS_SCALAR(Shininess, Material(), m_desc->MaterialAttributes.Shininess)
	EFFECT_MEMBER_AS_SCALAR(Refreactivity, Material(), m_desc->MaterialAttributes.Refreactivity)

	// Filter
	EFFECT_VARIABLE_AS_SCALAR(KernelDeltaX, m_desc->FilterAttributes.KernelDeltaX)
	EFFECT_VARIABLE_AS_SCALAR(KernelDeltaY, m_desc->FilterAttributes.KernelDeltaY)

private:
	ID3DX11Effect* m_effect = nullptr;
	const GMShaderVariablesDesc* m_desc = nullptr;
};

GMDx11EffectVariableBank& getVarBank()
{
	static GMDx11EffectVariableBank bank;
	return bank;
}

GMDx11CubeMapState& GMDx11Renderer::getCubeMapState()
{
	static GMDx11CubeMapState cms;
	return cms;
}

GMDx11Renderer::GMDx11Renderer()
{
	D(d);
	IShaderProgram* shaderProgram = getEngine()->getShaderProgram();
	shaderProgram->useProgram();
	GM_ASSERT(!d->effect);
	shaderProgram->getInterface(GameMachineInterfaceID::D3D11Effect, (void**)&d->effect);
	GM_ASSERT(d->effect);

	d->deviceContext = getEngine()->getDeviceContext();
	getVarBank().init(d->effect, getVariablesDesc());
}

void GMDx11Renderer::beginModel(GMModel* model, const GMGameObject* parent)
{
	D(d);
	IShaderProgram* shaderProgram = getEngine()->getShaderProgram();
	shaderProgram->useProgram();
	if (!d->inputLayout)
	{
		D3DX11_PASS_DESC passDesc;
		GM_DX_HR(getTechnique()->GetPassByIndex(0)->GetDesc(&passDesc));
		GM_DX_HR(getEngine()->getDevice()->CreateInputLayout(
			GMSHADER_ElementDescriptions,
			GM_array_size(GMSHADER_ElementDescriptions),
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&d->inputLayout
		));
	}

	// Renderer决定自己的顶点Layout
	d->deviceContext->IASetInputLayout(d->inputLayout);
	d->deviceContext->IASetPrimitiveTopology(getMode(model->getPrimitiveTopologyMode()));
	
	const GMShaderVariablesDesc* desc = getVariablesDesc();
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

	GMCamera& camera = GM.getCamera();
	if (camera.isDirty())
	{
		GMFloat4 viewPosition;
		camera.getLookAt().position.loadFloat4(viewPosition);
		shaderProgram->setVec4(desc->ViewPosition, viewPosition);
		shaderProgram->setMatrix4(desc->ViewMatrix, camera.getViewMatrix());
		shaderProgram->setMatrix4(desc->ProjectionMatrix, camera.getProjectionMatrix());
		shaderProgram->setMatrix4(desc->InverseViewMatrix, camera.getInverseViewMatrix());
		camera.cleanDirty();
	}

	const GMShadowSourceDesc& shadowSourceDesc = getEngine()->getShadowSourceDesc();
	GMDx11EffectVariableBank& bank = getVarBank();
	ID3DX11EffectScalarVariable* hasShadow = bank.HasShadow();
	if (shadowSourceDesc.type != GMShadowSourceDesc::NoShadow)
	{
		GM_DX_HR(hasShadow->SetBool(true));
	}
	else
	{
		GM_DX_HR(hasShadow->SetBool(false));
	}

	setGamma(shaderProgram);
}

void GMDx11Renderer::endModel()
{
}

void GMDx11Renderer::prepareScreenInfo()
{
	D(d);
	// 如果屏幕更改了，需要通知此处重新设置着色器
	if (!d->screenInfoPrepared)
	{
		GMDx11EffectVariableBank& bank = getVarBank();
		const GMGameMachineRunningStates& runningStates = GM.getGameMachineRunningStates();
		const auto& desc = getVariablesDesc();
		ID3DX11EffectVariable* screenInfo = d->effect->GetVariableByName(desc->ScreenInfoAttributes.ScreenInfo);
		GM_ASSERT(screenInfo->IsValid());

		ID3DX11EffectScalarVariable* screenWidth = bank.ScreenWidth();
		GM_DX_HR(screenWidth->SetInt(runningStates.renderRect.width));

		ID3DX11EffectScalarVariable* screenHeight = bank.ScreenHeight();
		GM_DX_HR(screenHeight->SetInt(runningStates.renderRect.height));

		ID3DX11EffectScalarVariable* multisampling = bank.ScreenMultiSampling();
		GM_DX_HR(multisampling->SetBool(runningStates.sampleCount > 1));
		d->screenInfoPrepared = true;
	}
}

void GMDx11Renderer::prepareTextures(GMModel* model)
{
	D(d);
	GM_FOREACH_ENUM_CLASS(type, GMTextureType::Ambient, GMTextureType::EndOfCommonTexture)
	{
		GMint count = GMMaxTextureCount(type);
		for (GMint i = 0; i < count; i++)
		{
			GMTextureFrames& textures = model->getShader().getTexture().getTextureFrames(type, i);
			// 写入纹理属性，如是否绘制，偏移等
			applyTextureAttribute(model, getTexture(textures), type, i);
		}
	}

	const GMDx11CubeMapState& cubeMapState = getCubeMapState();
	if (cubeMapState.hasCubeMap)
	{
		GM_ASSERT(cubeMapState.model && cubeMapState.cubeMapRenderer);
		cubeMapState.cubeMapRenderer->prepareTextures(cubeMapState.model);
	}
}

void GMDx11Renderer::setTextures(GMModel* model)
{
	D(d);
	GMint registerId = 0;
	GM_FOREACH_ENUM_CLASS(type, GMTextureType::Ambient, GMTextureType::EndOfCommonTexture)
	{
		GMint count = GMMaxTextureCount(type);
		for (GMint i = 0; i < count; ++i, ++registerId)
		{
			GMTextureFrames& textures = model->getShader().getTexture().getTextureFrames(type, i);

			// 获取序列中的这一帧
			ITexture* texture = getTexture(textures);
			if (texture)
			{
				// 激活动画序列
				texture->useTexture(&textures, registerId);
			}
		}
	}

	const GMDx11CubeMapState& cubeMapState = getCubeMapState();
	if (cubeMapState.hasCubeMap)
	{
		GM_ASSERT(cubeMapState.model && cubeMapState.cubeMapRenderer);
		cubeMapState.cubeMapRenderer->setTextures(cubeMapState.model);
	}
}

void GMDx11Renderer::applyTextureAttribute(GMModel* model, ITexture* texture, GMTextureType type, GMint index)
{
	D(d);
	const GMShaderVariablesDesc* desc = getVariablesDesc();
	ID3DX11EffectVariable* texAttrs = nullptr;
	const char* textureName = nullptr;
	if (type == GMTextureType::Ambient)
	{
		textureName = desc->AmbientTextureName;
	}
	else if (type == GMTextureType::Diffuse)
	{
		textureName = desc->DiffuseTextureName;
	}
	else if (type == GMTextureType::Specular)
	{
		textureName = desc->SpecularTextureName;
	}
	else if (type == GMTextureType::NormalMap)
	{
		textureName = desc->NormalMapTextureName;
	}
	else if (type == GMTextureType::Lightmap)
	{
		textureName = desc->LightMapTextureName;
	}
	else if (type == GMTextureType::CubeMap)
	{
		textureName = desc->CubeMapTextureName;
	}
	else
	{
		return;
	}
	GM_ASSERT(textureName);

	texAttrs = d->effect->GetVariableByName(textureName)->GetElement(index);
	CHECK_VAR(texAttrs);

	if (texture)
	{
		ID3DX11EffectScalarVariable* enabled = texAttrs->GetMemberByName(desc->TextureAttributes.Enabled)->AsScalar();
		CHECK_VAR(enabled);
		GM_DX_HR(enabled->SetBool(TRUE));

		ID3DX11EffectScalarVariable* offsetX = texAttrs->GetMemberByName(desc->TextureAttributes.OffsetX)->AsScalar();
		ID3DX11EffectScalarVariable* offsetY = texAttrs->GetMemberByName(desc->TextureAttributes.OffsetY)->AsScalar();
		ID3DX11EffectScalarVariable* scaleX = texAttrs->GetMemberByName(desc->TextureAttributes.ScaleX)->AsScalar();
		ID3DX11EffectScalarVariable* scaleY = texAttrs->GetMemberByName(desc->TextureAttributes.ScaleY)->AsScalar();
		CHECK_VAR(offsetX);
		CHECK_VAR(offsetY);
		CHECK_VAR(scaleX);
		CHECK_VAR(scaleY);
		GM_DX_HR(offsetX->SetFloat(1.f));
		GM_DX_HR(offsetY->SetFloat(1.f));
		GM_DX_HR(scaleX->SetFloat(1.f));
		GM_DX_HR(scaleY->SetFloat(1.f));

		auto applyCallback = [&](GMS_TextureModType type, Pair<GMfloat, GMfloat>&& args) {
			if (type == GMS_TextureModType::SCALE)
			{
				GM_DX_HR(scaleX->SetFloat(args.first));
				GM_DX_HR(scaleY->SetFloat(args.second));
			}
			else if (type == GMS_TextureModType::SCROLL)
			{
				GM_DX_HR(offsetX->SetFloat(args.first));
				GM_DX_HR(offsetY->SetFloat(args.second));
			}
			else
			{
				GM_ASSERT(false);
			}
		};

		model->getShader().getTexture().getTextureFrames(type, index).applyTexMode(GM.getGameTimeSeconds(), applyCallback);
	}
	else
	{
		// 将这个Texture的Enabled设置为false
		ID3DX11EffectScalarVariable* enabled = texAttrs->GetMemberByName(desc->TextureAttributes.Enabled)->AsScalar();
		CHECK_VAR(enabled);
		enabled->SetBool(FALSE);
	}
}

void GMDx11Renderer::prepareBuffer(GMModel* model)
{
	D(d);
	GMuint stride = sizeof(GMVertex);
	GMuint offset = 0;
	GMComPtr<ID3D11Buffer> vertexBuffer;
	IQueriable* modelDataProxy = model->getModelDataProxy();
	modelDataProxy->getInterface(GameMachineInterfaceID::D3D11VertexBuffer, (void**)&vertexBuffer);
	GM_ASSERT(vertexBuffer);
	GM_DX11_SET_OBJECT_NAME_A(vertexBuffer, "GM_VERTEX_BUFFER");
	d->deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	if (model->getDrawMode() == GMModelDrawMode::Index)
	{
		GMComPtr<ID3D11Buffer> indexBuffer;
		modelDataProxy->getInterface(GameMachineInterfaceID::D3D11IndexBuffer, (void**)&indexBuffer);
		GM_ASSERT(indexBuffer);
		d->deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	}
}

void GMDx11Renderer::prepareLights()
{
	D(d);
	getEngine()->activateLights(this);
}

void GMDx11Renderer::prepareRasterizer(GMModel* model)
{
	D(d);
	bool multisampleEnable = GM.getGameMachineRunningStates().sampleCount > 1;
	if (!d->rasterizer)
	{
		const GMShaderVariablesDesc& svd = getEngine()->getShaderProgram()->getDesc();
		d->rasterizer = d->effect->GetVariableByName(svd.RasterizerState)->AsRasterizer();
	}

	GMDx11RasterizerStates& rasterStates = GMDx11RasterizerStates::instance();
	GM_ASSERT(d->rasterizer);
	GM_DX_HR(d->rasterizer->SetRasterizerState(
		0, 
		rasterStates.getRasterStates(model->getShader().getFrontFace(), model->getShader().getCull())
	));
}

void GMDx11Renderer::prepareMaterials(GMModel* model)
{
	D(d);
	GMDx11EffectVariableBank& bank = getVarBank();
	ID3D11DeviceContext* context = d->deviceContext;
	const GMMaterial& material = model->getShader().getMaterial();
	GM_DX_HR(bank.Ka()->SetFloatVector(ValuePointer(material.ka)));
	GM_DX_HR(bank.Kd()->SetFloatVector(ValuePointer(material.kd)));
	GM_DX_HR(bank.Ks()->SetFloatVector(ValuePointer(material.ks)));
	GM_DX_HR(bank.Shininess()->SetFloat(material.shininess));
	GM_DX_HR(bank.Refreactivity()->SetFloat(material.refractivity));
}

void GMDx11Renderer::prepareBlend(GMModel* model)
{
	D(d);
	if (!d->blend)
	{
		const GMShaderVariablesDesc& svd = getEngine()->getShaderProgram()->getDesc();
		d->blend = d->effect->GetVariableByName(svd.BlendState)->AsBlend();
	}
	GM_ASSERT(d->blend);

	const GMDx11GlobalBlendStateDesc& globalBlendState = getEngine()->getGlobalBlendState();
	GMDx11BlendStates& blendStates = GMDx11BlendStates::instance();
	if (globalBlendState.enabled)
	{
		// 全局blend开启时
		if (model->getShader().getBlend())
		{
			GM_DX_HR(d->blend->SetBlendState(
				0,
				blendStates.getBlendState(true, model->getShader().getBlendFactorSource(), model->getShader().getBlendFactorDest())
			));
		}
		else
		{
			GM_DX_HR(d->blend->SetBlendState(
				0,
				blendStates.getBlendState(true, globalBlendState.source, globalBlendState.dest)
			));
		}
	}
	else
	{
		// 全局blend关闭，此时应该应用正在绘制物体的Blend状态
		if (model->getShader().getBlend())
		{
			GM_DX_HR(d->blend->SetBlendState(
				0,
				blendStates.getBlendState(true, model->getShader().getBlendFactorSource(), model->getShader().getBlendFactorDest())
			));
		}
		else
		{
			GM_DX_HR(d->blend->SetBlendState(
				0,
				blendStates.getDisabledBlendState()
			));
		}
	}
}

void GMDx11Renderer::prepareDepthStencil(GMModel* model)
{
	D(d);
	if (!d->depthStencil)
	{
		const GMShaderVariablesDesc& svd = getEngine()->getShaderProgram()->getDesc();
		d->depthStencil = d->effect->GetVariableByName(svd.DepthStencilState)->AsDepthStencil();
	}
	GM_ASSERT(d->depthStencil);

	GMDx11DepthStencilStates& depthStencilStates = GMDx11DepthStencilStates::instance();
	GM_DX_HR(d->depthStencil->SetDepthStencilState(
		0,
		depthStencilStates.getDepthStencilState(
			!model->getShader().getNoDepthTest(),
			getEngine()->getStencilOptions()
		)
	));
}

ITexture* GMDx11Renderer::getTexture(GMTextureFrames& frames)
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

void GMDx11Renderer::setGamma(IShaderProgram* shaderProgram)
{
	D(d);
	static const GMShaderVariablesDesc* desc = getVariablesDesc();
	bool needGammaCorrection = getEngine()->needGammaCorrection();
	shaderProgram->setBool(desc->GammaCorrection.GammaCorrection, needGammaCorrection);
	if (needGammaCorrection)
	{
		GMfloat gamma = getEngine()->getGammaValue();
		if (gamma != d->gamma)
		{
			shaderProgram->setFloat(desc->GammaCorrection.GammaValue, gamma);
			d->gamma = gamma;
		}
	}
}

void GMDx11Renderer::draw(GMModel* model)
{
	D(d);
	prepareScreenInfo();
	prepareBuffer(model);
	prepareLights();
	prepareMaterials(model);
	prepareRasterizer(model);
	prepareBlend(model);
	prepareDepthStencil(model);
	passAllAndDraw(model);
}

void GMDx11Renderer::passAllAndDraw(GMModel* model)
{
	D(d);
	D3DX11_TECHNIQUE_DESC techDesc;
	GM_DX_HR(getTechnique()->GetDesc(&techDesc));

	for (GMuint p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = getTechnique()->GetPassByIndex(p);
		prepareTextures(model);
		pass->Apply(0, d->deviceContext);
		setTextures(model);

		GM_ASSERT(model->getVerticesCount() < std::numeric_limits<UINT>::max());
		if (model->getDrawMode() == GMModelDrawMode::Vertex)
			d->deviceContext->Draw((UINT)model->getVerticesCount(), 0);
		else
			d->deviceContext->DrawIndexed((UINT)model->getVerticesCount(), 0, 0);
	}
}

ID3DX11EffectTechnique* GMDx11Renderer::getTechnique()
{
	D(d);
	if (!d->technique)
	{
		GMComPtr<ID3DX11Effect> effect;
		IShaderProgram* shaderProgram = getEngine()->getShaderProgram();
		shaderProgram->getInterface(GameMachineInterfaceID::D3D11Effect, (void**)&effect);
		GM_ASSERT(effect);

		d->technique = effect->GetTechniqueByName(getTechniqueName());
		GM_ASSERT(d->technique);
	}
	return d->technique;
}

void GMDx11Renderer_CubeMap::prepareTextures(GMModel* model)
{
	GMTextureFrames& textures = model->getShader().getTexture().getTextureFrames(GMTextureType::CubeMap, 0);
	// 写入纹理属性，如是否绘制，偏移等
	applyTextureAttribute(model, getTexture(textures), GMTextureType::CubeMap, 0);
}

void GMDx11Renderer_CubeMap::setTextures(GMModel* model)
{
	enum {
		Register = GMTextureRegisterQuery<GMTextureType::CubeMap>::Value
	};

	GMTextureFrames& textures = model->getShader().getTexture().getTextureFrames(GMTextureType::CubeMap, 0);

	// 获取序列中的这一帧
	ITexture* texture = getTexture(textures);
	if (texture)
	{
		// 激活动画序列
		texture->useTexture(&textures, Register);
		GMDx11CubeMapState& cubeMapState = getCubeMapState();
		if (cubeMapState.model != model)
		{
			cubeMapState.hasCubeMap = true;
			cubeMapState.cubeMapRenderer = this;
			cubeMapState.model = model;
		}
	}
}

void GMDx11Renderer_Filter::draw(GMModel* model)
{
	D(d);
	prepareScreenInfo();
	prepareBuffer(model);
	prepareRasterizer(model);
	prepareBlend(model);
	prepareDepthStencil(model);
	passAllAndDraw(model);
}

void GMDx11Renderer_Filter::passAllAndDraw(GMModel* model)
{
	D(d);
	D3DX11_TECHNIQUE_DESC techDesc;
	GM_DX_HR(getTechnique()->GetDesc(&techDesc));

	for (GMuint p = 0; p < techDesc.Passes; ++p)
	{
		GMDx11Texture* filterTexture = gm_cast<GMDx11Texture*>(model->getShader().getTexture().getTextureFrames(GMTextureType::Ambient, 0).getFrameByIndex(0));
		GM_ASSERT(filterTexture);
		ID3DX11EffectPass* pass = getTechnique()->GetPassByIndex(p);
		if (GM.getGameMachineRunningStates().sampleCount == 1)
		{
			GM_DX_HR(d->effect->GetVariableByName("FilterTexture")->AsShaderResource()->SetResource(filterTexture->getResourceView()));
		}
		else
		{
			GM_DX_HR(d->effect->GetVariableByName("FilterTexture_MSAA")->AsShaderResource()->SetResource(filterTexture->getResourceView()));
		}
		pass->Apply(0, d->deviceContext);

		GM_ASSERT(model->getVerticesCount() < std::numeric_limits<UINT>::max());
		if (model->getDrawMode() == GMModelDrawMode::Vertex)
			d->deviceContext->Draw((UINT)model->getVerticesCount(), 0);
		else
			d->deviceContext->DrawIndexed((UINT)model->getVerticesCount(), 0, 0);
	}
}

void GMDx11Renderer_Filter::beginModel(GMModel* model, const GMGameObject* parent)
{
	D(d);
	GMDx11Renderer::beginModel(model, parent);

	GMDx11EffectVariableBank& bank = getVarBank();
	ID3DX11EffectScalarVariable* kernelDeltaX = bank.KernelDeltaX();
	ID3DX11EffectScalarVariable* kernelDeltaY = bank.KernelDeltaY();
	GM_ASSERT(kernelDeltaX->IsValid() && kernelDeltaY->IsValid());
	GMFloat4 delta;
	getEngine()->getCurrentFilterKernelDelta().loadFloat4(delta);
	GM_DX_HR(kernelDeltaX->SetInt(delta[0]));
	GM_DX_HR(kernelDeltaY->SetInt(delta[1]));

	GMFilterMode::Mode filterMode = getEngine()->getCurrentFilterMode();
	IShaderProgram* shaderProgram = getEngine()->getShaderProgram();
	const GMShaderVariablesDesc* desc = getVariablesDesc();
	bool b = shaderProgram->setInterfaceInstance(desc->FilterAttributes.Filter, desc->FilterAttributes.Types[filterMode], GMShaderType::Effect);
	GM_ASSERT(b);

	if (getEngine()->needHDR())
	{
		// 其实Filter和3D的着色器是同一个，因此这里本身不需要再次设置Gamma
		// 不过考虑到之后可能是分开的着色器程序，先设置上，开销也不大
		setGamma(shaderProgram);
		setHDR();
	}
}

void GMDx11Renderer_Filter::setHDR()
{

}

void GMDx11Renderer_Deferred_3D::passAllAndDraw(GMModel* model)
{
	D(d);
	D3DX11_TECHNIQUE_DESC techDesc;
	GM_DX_HR(getTechnique()->GetDesc(&techDesc));

	IGBuffer* gbuffer = getEngine()->getGBuffer();
	IFramebuffers* framebuffers = gbuffer->getGeometryFramebuffers();
	for (GMuint p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = getTechnique()->GetPassByIndex(p);
		prepareTextures(model);
		pass->Apply(0, d->deviceContext);
		setTextures(model);

		GM_ASSERT(framebuffers);
		framebuffers->bind();
		GM_ASSERT(model->getVerticesCount() < std::numeric_limits<UINT>::max());
		if (model->getDrawMode() == GMModelDrawMode::Vertex)
			d->deviceContext->Draw((UINT)model->getVerticesCount(), 0);
		else
			d->deviceContext->DrawIndexed((UINT)model->getVerticesCount(), 0, 0);
		framebuffers->unbind();
	}
}

void GMDx11Renderer_Deferred_3D_LightPass::passAllAndDraw(GMModel* model)
{
	D(d);
	D3DX11_TECHNIQUE_DESC techDesc;
	GM_DX_HR(getTechnique()->GetDesc(&techDesc));

	for (GMuint p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = getTechnique()->GetPassByIndex(p);
		setDeferredTexturesBeforeApply();
		pass->Apply(0, d->deviceContext);
		const GMDx11CubeMapState& cubeMapState = getCubeMapState();
		if (cubeMapState.hasCubeMap)
		{
			GM_ASSERT(cubeMapState.model && cubeMapState.cubeMapRenderer);
			cubeMapState.cubeMapRenderer->setTextures(cubeMapState.model);
		}

		GM_ASSERT(model->getVerticesCount() < std::numeric_limits<UINT>::max());
		if (model->getDrawMode() == GMModelDrawMode::Vertex)
			d->deviceContext->Draw((UINT)model->getVerticesCount(), 0);
		else
			d->deviceContext->DrawIndexed((UINT)model->getVerticesCount(), 0, 0);
	}
}

void GMDx11Renderer_Deferred_3D_LightPass::setDeferredTexturesBeforeApply()
{
	D(d);
	GMDx11GBuffer* gbuffer = gm_cast<GMDx11GBuffer*>(getEngine()->getGBuffer());
	gbuffer->useGeometryTextures(d->effect);

	const GMDx11CubeMapState& cubeMapState = getCubeMapState();
	if (cubeMapState.hasCubeMap)
	{
		GM_ASSERT(cubeMapState.model && cubeMapState.cubeMapRenderer);
		cubeMapState.cubeMapRenderer->prepareTextures(cubeMapState.model);
	}
}

void GMDx11Renderer_3D_Shadow::beginModel(GMModel* model, const GMGameObject* parent)
{
	D(d);
	IShaderProgram* shaderProgram = getEngine()->getShaderProgram();
	shaderProgram->useProgram();
	if (!d->inputLayout)
	{
		D3DX11_PASS_DESC passDesc;
		GM_DX_HR(getTechnique()->GetPassByIndex(0)->GetDesc(&passDesc));
		GM_DX_HR(getEngine()->getDevice()->CreateInputLayout(
			GMSHADER_ElementDescriptions,
			GM_array_size(GMSHADER_ElementDescriptions),
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&d->inputLayout
		));
	}

	// Renderer决定自己的顶点Layout
	d->deviceContext->IASetInputLayout(d->inputLayout);
	d->deviceContext->IASetPrimitiveTopology(getMode(model->getPrimitiveTopologyMode()));

	const GMShaderVariablesDesc* desc = getVariablesDesc();
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

	const GMShadowSourceDesc& shadowSourceDesc = getEngine()->getShadowSourceDesc();
	const GMCamera& camera = shadowSourceDesc.camera;
	GMFloat4 viewPosition;
	shadowSourceDesc.position.loadFloat4(viewPosition);

	GMDx11EffectVariableBank& bank = getVarBank();
	ID3DX11EffectVariable* shadowInfo = bank.ShadowInfo();
	ID3DX11EffectVectorVariable* position = bank.ShadowPosition();
	ID3DX11EffectMatrixVariable* shadowMatrix = bank.ShadowMatrix();
	ID3DX11EffectScalarVariable* shadowMapWidth = bank.ShadowMapWidth();
	ID3DX11EffectScalarVariable* shadowMapHeight = bank.ShadowMapHeight();
	ID3DX11EffectScalarVariable* biasMin = bank.ShadowBiasMin();
	ID3DX11EffectScalarVariable* biasMax = bank.ShadowBiasMax();
	GM_DX_HR(position->SetFloatVector(ValuePointer(viewPosition)));
	GM_DX_HR(shadowMatrix->SetMatrix(ValuePointer(camera.getViewMatrix() * camera.getProjectionMatrix())));
	GM_DX_HR(biasMin->SetFloat(shadowSourceDesc.biasMin));
	GM_DX_HR(biasMax->SetFloat(shadowSourceDesc.biasMax));

	GMDx11ShadowFramebuffers* shadowFramebuffers = gm_cast<GMDx11ShadowFramebuffers*>(getEngine()->getShadowMapFramebuffers());
	GM_DX_HR(shadowMapWidth->SetInt(shadowFramebuffers->getShadowMapWidth()));
	GM_DX_HR(shadowMapHeight->SetInt(shadowFramebuffers->getShadowMapHeight()));

	ID3DX11EffectShaderResourceVariable* shadowMap = GM.getGameMachineRunningStates().sampleCount > 1 ? bank.ShadowMapMSAA() : bank.ShadowMap();
	GM_DX_HR(shadowMap->SetResource(shadowFramebuffers->getShadowMapShaderResourceView()));
}