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

	class GMDx11WhiteTexture : public GMDx11Texture
	{

	public:
		GMDx11WhiteTexture()
			: GMDx11Texture(nullptr)
		{
		}

		virtual void init() override
		{
			static GMbyte texData[] = { 0xFF, 0xFF, 0xFF, 0xFF };
			// 新建一个RGBA白色纹理
			D(d);
			D3D11_TEXTURE2D_DESC texDesc = { 0 };
			D3D11_SUBRESOURCE_DATA resourceData;
			GMComPtr<ID3D11Texture2D> texture;

			texDesc.Width = 1;
			texDesc.Height = 1;
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.Usage = D3D11_USAGE_DEFAULT;
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			texDesc.CPUAccessFlags = 0;
			texDesc.MiscFlags = 0;

			GMint index = 0;
			GM_ASSERT(texDesc.ArraySize == 1);
			GMuint pitch = 1;
			resourceData.pSysMem = texData;
			resourceData.SysMemPitch = pitch * 4;

			GM_DX_HR(d->device->CreateTexture2D(&texDesc, &resourceData, &texture));
			d->resource = texture;

			GM_ASSERT(d->resource);
			GM_DX_HR(d->device->CreateShaderResourceView(
				d->resource,
				NULL,
				&d->shaderResourceView
			));
		}

		virtual void bindSampler(GMTextureSampler*)
		{
			D(d);
			if (!d->samplerState)
			{
				// 创建采样器
				D3D11_SAMPLER_DESC desc = GMDx11Helper::GMGetDx11DefaultSamplerDesc();
				desc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_POINT;
				desc.AddressU = desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
				GM_DX_HR(d->device->CreateSamplerState(&desc, &d->samplerState));
			}
		}
	};

	ITexture* getWhiteTexture()
	{
		static bool s_inited = false;
		static GMDx11WhiteTexture s_texture;
		if (!s_inited)
		{
			s_texture.init();
			s_inited = true;
		}
		return &s_texture;
	}
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
		GMTextureSampler& sampler = model->getShader().getTextureList().getTextureSampler(type);
		// 写入纹理属性，如是否绘制，偏移等
		ITexture* texture = getTexture(sampler);
		applyTextureAttribute(model, texture, type);
		if (texture)
		{
			// 激活动画序列
			texture->bindSampler(&sampler);
			texture->useTexture((GMint)type);
		}
		else if (
			type == GMTextureType::Ambient ||
			type == GMTextureType::Diffuse ||
			type == GMTextureType::Specular ||
			type == GMTextureType::Lightmap
			)
		{
			ITexture* whiteTexture = getWhiteTexture();
			applyTextureAttribute(model, whiteTexture, type);
			whiteTexture->bindSampler(nullptr);
			whiteTexture->useTexture((GMint)type);
		}
	}

	const GMDx11CubeMapState& cubeMapState = getCubeMapState();
	if (cubeMapState.hasCubeMap)
	{
		GM_ASSERT(cubeMapState.model && cubeMapState.cubeMapRenderer);
		cubeMapState.cubeMapRenderer->prepareTextures(cubeMapState.model);
	}
}

void GMDx11Renderer::applyTextureAttribute(GMModel* model, ITexture* texture, GMTextureType type)
{
	D(d);
	const GMShaderVariablesDesc* desc = getVariablesDesc();
	const char* textureName = nullptr;
	switch (type)
	{
	case GMTextureType::Ambient:
		textureName = desc->AmbientTextureName;
		break;
	case GMTextureType::Diffuse:
		textureName = desc->DiffuseTextureName;
		break;
	case GMTextureType::Specular:
		textureName = desc->SpecularTextureName;
		break;
	case GMTextureType::NormalMap:
		textureName = desc->NormalMapTextureName;
		break;
	case GMTextureType::Lightmap:
		textureName = desc->LightMapTextureName;
		break;
	case GMTextureType::CubeMap:
		textureName = desc->CubeMapTextureName;
		break;
	case GMTextureType::Albedo:
		textureName = desc->AlbedoTextureName;
		break;
	case GMTextureType::MetallicRoughnessAO:
		textureName = desc->MetallicRoughnessAOTextureName;
		break;
	default:
		GM_ASSERT(false);
		return;
	}
	GM_ASSERT(textureName);

	ID3DX11EffectVariable* textureAttribute = nullptr;
	{
		auto iter = d->textureAttributes.find(textureName);
		if (iter != d->textureAttributes.end())
		{
			textureAttribute = iter->second;
		}
		else
		{
			textureAttribute = d->effect->GetVariableByName(textureName);
			CHECK_VAR(textureAttribute);
			d->textureAttributes[textureName] = textureAttribute;
		}
	}

	const GMTextureAttributeBank* bank = nullptr;
	{
		auto iter = d->textureVariables.find(textureName);
		if (iter != d->textureVariables.end())
		{
			bank = &iter->second;
		}
		else
		{
			GMTextureAttributeBank newBank;
			newBank.enabled = textureAttribute->GetMemberByName(desc->TextureAttributes.Enabled)->AsScalar();
			newBank.offsetX = textureAttribute->GetMemberByName(desc->TextureAttributes.OffsetX)->AsScalar();
			newBank.offsetY = textureAttribute->GetMemberByName(desc->TextureAttributes.OffsetY)->AsScalar();
			newBank.scaleX = textureAttribute->GetMemberByName(desc->TextureAttributes.ScaleX)->AsScalar();
			newBank.scaleY = textureAttribute->GetMemberByName(desc->TextureAttributes.ScaleY)->AsScalar();
			CHECK_VAR(newBank.enabled);
			CHECK_VAR(newBank.offsetX);
			CHECK_VAR(newBank.offsetY);
			CHECK_VAR(newBank.scaleX);
			CHECK_VAR(newBank.scaleY);
			d->textureVariables[textureName] = newBank;
			bank = &d->textureVariables[textureName];
		}
	}

	if (texture)
	{
		GM_DX_HR(bank->enabled->SetBool(TRUE));
		GM_DX_HR(bank->offsetX->SetFloat(1.f));
		GM_DX_HR(bank->offsetY->SetFloat(1.f));
		GM_DX_HR(bank->scaleX->SetFloat(1.f));
		GM_DX_HR(bank->scaleY->SetFloat(1.f));

		auto applyCallback = [&](GMS_TextureModType type, Pair<GMfloat, GMfloat>&& args) {
			if (type == GMS_TextureModType::SCALE)
			{
				GM_DX_HR(bank->scaleX->SetFloat(args.first));
				GM_DX_HR(bank->scaleY->SetFloat(args.second));
			}
			else if (type == GMS_TextureModType::SCROLL)
			{
				GM_DX_HR(bank->offsetX->SetFloat(args.first));
				GM_DX_HR(bank->offsetY->SetFloat(args.second));
			}
			else
			{
				GM_ASSERT(false);
			}
		};

		model->getShader().getTextureList().getTextureSampler(type).applyTexMode(GM.getGameTimeSeconds(), applyCallback);
	}
	else
	{
		// 将这个Texture的Enabled设置为false
		GM_DX_HR(bank->enabled->SetBool(FALSE));
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
	const GMShader& shader = model->getShader();
	const GMMaterial& material = shader.getMaterial();
	GM_DX_HR(bank.Ka()->SetFloatVector(ValuePointer(material.ka)));
	GM_DX_HR(bank.Kd()->SetFloatVector(ValuePointer(material.kd)));
	GM_DX_HR(bank.Ks()->SetFloatVector(ValuePointer(material.ks)));
	GM_DX_HR(bank.Shininess()->SetFloat(material.shininess));
	GM_DX_HR(bank.Refreactivity()->SetFloat(material.refractivity));

	const GMShaderVariablesDesc* desc = getVariablesDesc();
	IShaderProgram* shaderProgram = getEngine()->getShaderProgram();
	GMIlluminationModel illuminationModel = shader.getIlluminationModel();
	if (illuminationModel == GMIlluminationModel::Phong)
		shaderProgram->setInterfaceInstance(desc->IlluminationModel, "GM_Phong", GMShaderType::Effect);
	else if (illuminationModel == GMIlluminationModel::CookTorranceBRDF)
		shaderProgram->setInterfaceInstance(desc->IlluminationModel, "GM_CookTorranceBRDF", GMShaderType::Effect);
	else
		GM_ASSERT(false);
}

void GMDx11Renderer::prepareBlend(GMModel* model)
{
	D(d);
	if (!d->blend)
	{
		const GMShaderVariablesDesc* svd = getVariablesDesc();
		d->blend = d->effect->GetVariableByName(svd->BlendState)->AsBlend();
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

ITexture* GMDx11Renderer::getTexture(GMTextureSampler& sampler)
{
	D(d);
	if (sampler.getFrameCount() == 0)
		return nullptr;

	if (sampler.getFrameCount() == 1)
		return sampler.getFrameByIndex(0);

	// 如果frameCount > 1，说明是个动画，要根据Shader的间隔来选择合适的帧
	// TODO
	GMint elapsed = GM.getGameTimeSeconds() * 1000;

	return sampler.getFrameByIndex((elapsed / sampler.getAnimationMs()) % sampler.getFrameCount());
}

void GMDx11Renderer::setGamma(IShaderProgram* shaderProgram)
{
	D(d);
	static const GMShaderVariablesDesc* desc = getVariablesDesc();
	bool needGammaCorrection = getEngine()->needGammaCorrection();
	shaderProgram->setBool(desc->GammaCorrection.GammaCorrection, needGammaCorrection);
	GMfloat gamma = getEngine()->getGammaValue();
	if (gamma != d->gamma)
	{
		shaderProgram->setFloat(desc->GammaCorrection.GammaInvValue, 1.f / gamma);
		d->gamma = gamma;
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
	prepareTextures(model);
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
		pass->Apply(0, d->deviceContext);
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

void GMDx11Renderer_2D::prepareTextures(GMModel* model)
{
	D(d);
	GM_FOREACH_ENUM_CLASS(type, GMTextureType::Ambient, GMTextureType::EndOfCommonTexture)
	{
		GMTextureSampler& sampler = model->getShader().getTextureList().getTextureSampler(type);
		// 写入纹理属性，如是否绘制，偏移等
		ITexture* texture = getTexture(sampler);
		applyTextureAttribute(model, texture, type);
		if (texture)
		{
			texture->bindSampler(&model->getShader().getTextureList().getTextureSampler(type));
			texture->useTexture((GMint)type);
		}
	}
}

void GMDx11Renderer_CubeMap::prepareTextures(GMModel* model)
{
	GMTextureSampler& sampler = model->getShader().getTextureList().getTextureSampler(GMTextureType::CubeMap);
	// 写入纹理属性，如是否绘制，偏移等
	ITexture* texture = getTexture(sampler);
	applyTextureAttribute(model, texture, GMTextureType::CubeMap);
	if (texture)
	{
		texture->bindSampler(&model->getShader().getTextureList().getTextureSampler(GMTextureType::CubeMap));
		texture->useTexture((GMint)GMTextureType::CubeMap);
		GMDx11CubeMapState& cubeMapState = getCubeMapState();
		if (cubeMapState.model != model)
		{
			cubeMapState.hasCubeMap = true;
			cubeMapState.cubeMapRenderer = this;
			cubeMapState.model = model;
		}
	}
}

GMDx11Renderer_Filter::GMDx11Renderer_Filter()
{
	setHDR(getEngine()->getShaderProgram());
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
	D_BASE(d, Base);
	D3DX11_TECHNIQUE_DESC techDesc;
	GM_DX_HR(getTechnique()->GetDesc(&techDesc));

	for (GMuint p = 0; p < techDesc.Passes; ++p)
	{
		GMDx11Texture* filterTexture = gm_cast<GMDx11Texture*>(model->getShader().getTextureList().getTextureSampler(GMTextureType::Ambient).getFrameByIndex(0));
		GM_ASSERT(filterTexture);
		ID3DX11EffectPass* pass = getTechnique()->GetPassByIndex(p);
		if (GM.getGameMachineRunningStates().sampleCount == 1)
		{
			GM_DX_HR(d->effect->GetVariableByName("GM_FilterTexture")->AsShaderResource()->SetResource(filterTexture->getResourceView()));
		}
		else
		{
			GM_DX_HR(d->effect->GetVariableByName("GM_FilterTexture_MSAA")->AsShaderResource()->SetResource(filterTexture->getResourceView()));
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
	static const GMShaderVariablesDesc* desc = getVariablesDesc();
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
	bool b = shaderProgram->setInterfaceInstance(desc->FilterAttributes.Filter, desc->FilterAttributes.Types[filterMode], GMShaderType::Effect);
	GM_ASSERT(b);

	if (d->state.HDR != getEngine()->needHDR() || d->state.toneMapping != getEngine()->getToneMapping())
	{
		d->state.HDR = getEngine()->needHDR();
		d->state.toneMapping = getEngine()->getToneMapping();
		if (d->state.HDR)
		{
			setGamma(shaderProgram);
			setHDR(shaderProgram);
		}
		else
		{
			shaderProgram->setBool(desc->HDR.HDR, false);
		}
	}
}

void GMDx11Renderer_Filter::setHDR(IShaderProgram* shaderProgram)
{
	static const GMShaderVariablesDesc* desc = getVariablesDesc();
	D(d);
	shaderProgram->setBool(desc->HDR.HDR, true);
	if (d->state.toneMapping == GMToneMapping::Reinhard)
	{
		bool b = shaderProgram->setInterfaceInstance(desc->HDR.ToneMapping, "ReinhardToneMapping", GMShaderType::Effect);
		GM_ASSERT(b);
	}
	else
	{
		GM_ASSERT(false);
		gm_warning("Invalid tonemapping.");
	}
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
		pass->Apply(0, d->deviceContext);

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
	setDeferredTexturesBeforeApply();

	D3DX11_TECHNIQUE_DESC techDesc;
	GM_DX_HR(getTechnique()->GetDesc(&techDesc));
	for (GMuint p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = getTechnique()->GetPassByIndex(p);
		pass->Apply(0, d->deviceContext);
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