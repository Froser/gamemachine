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
		case (GMS_BlendFunc::ONE_MINUS_SRC_COLOR):
			return D3D11_BLEND_INV_SRC_COLOR;
		case (GMS_BlendFunc::ONE_MINUS_DST_COLOR):
			return D3D11_BLEND_INV_DEST_COLOR;
		default:
			GM_ASSERT(false);
			return D3D11_BLEND_ONE;
		}
	}

	inline D3D11_BLEND_OP toDx11BlendOp(GMS_BlendOp op)
	{
		D3D11_BLEND_OP blendOp = D3D11_BLEND_OP_ADD;
		if (op == GMS_BlendOp::ADD)
		{
			blendOp = D3D11_BLEND_OP_ADD;
		}
		else if (op == GMS_BlendOp::SUBSTRACT)
		{
			blendOp = D3D11_BLEND_OP_SUBTRACT;
		}
		else if (op == GMS_BlendOp::REVERSE_SUBSTRACT)
		{
			blendOp = D3D11_BLEND_OP_REV_SUBTRACT;
		}
		else
		{
			GM_ASSERT(false);
			gm_error(L"Invalid blend op");
			blendOp = D3D11_BLEND_OP_ADD;
		}
		return blendOp;
	}

	D3D11_BLEND_DESC getBlendDesc(
		bool enabled,
		GMS_BlendFunc sourceRGB,
		GMS_BlendFunc destRGB,
		GMS_BlendOp opRGB,
		GMS_BlendFunc sourceAlpha,
		GMS_BlendFunc destAlpha,
		GMS_BlendOp opAlpha
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
			renderTarget.SrcBlend = toDx11Blend(sourceRGB);
			renderTarget.SrcBlendAlpha = toDx11Blend(sourceAlpha);
			renderTarget.DestBlend = toDx11Blend(destRGB);
			renderTarget.DestBlendAlpha = toDx11Blend(destAlpha);
			renderTarget.BlendOp = toDx11BlendOp(opRGB);
			renderTarget.BlendOpAlpha = toDx11BlendOp(opAlpha);
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

	ITexture* createWhiteTexture(const IRenderContext* context)
	{
		ITexture* texture = nullptr;
		GM.getFactory()->createWhiteTexture(context, &texture);
		texture->init();
		return texture;
	}
}

#define EFFECT_VARIABLE(funcName, name) \
	private: ID3DX11EffectVariable* effect_var_##funcName = nullptr;				\
	public: ID3DX11EffectVariable* funcName() {										\
		ID3DX11EffectVariable* effect_var_##funcName = nullptr;						\
		if (!effect_var_##funcName) {												\
			effect_var_##funcName = m_effect->GetVariableByName(name);				\
			GM_ASSERT(effect_var_##funcName->IsValid()); }							\
		return effect_var_##funcName;												\
	}

#define EFFECT_VARIABLE_AS(funcName, name, retType, to) \
	private: retType* effect_var_##funcName = nullptr;								\
	public: retType* funcName() {													\
		retType* effect_var_##funcName = nullptr;									\
		if (!effect_var_##funcName) {												\
			effect_var_##funcName = m_effect->GetVariableByName(name)->to();		\
			GM_ASSERT(effect_var_##funcName->IsValid()); }							\
		return effect_var_##funcName;												\
	}

#define EFFECT_VARIABLE_AS_SHADER_RESOURCE(funcName, name) \
	EFFECT_VARIABLE_AS(funcName, name, ID3DX11EffectShaderResourceVariable, AsShaderResource)

#define EFFECT_VARIABLE_AS_SCALAR(funcName, name) \
	EFFECT_VARIABLE_AS(funcName, name, ID3DX11EffectScalarVariable, AsScalar)

#define EFFECT_MEMBER_AS(funcName, effect, name, retType, to) \
	private: retType* effect_var_##funcName = nullptr;								\
	public: retType* funcName() {													\
		retType* effect_var_##funcName = nullptr;									\
		if (!effect_var_##funcName) {												\
			effect_var_##funcName = effect->GetMemberByName(name)->to();			\
			GM_ASSERT(effect_var_##funcName->IsValid()); }							\
		return effect_var_##funcName;												\
	}

#define EFFECT_MEMBER_AS_SCALAR(funcName, effect, name) \
	EFFECT_MEMBER_AS(funcName, effect, name, ID3DX11EffectScalarVariable, AsScalar)

#define EFFECT_MEMBER_AS_MATRIX(funcName, effect, name) \
	EFFECT_MEMBER_AS(funcName, effect, name, ID3DX11EffectMatrixVariable, AsMatrix)

#define EFFECT_MEMBER_AS_VECTOR(funcName, effect, name) \
	EFFECT_MEMBER_AS(funcName, effect, name, ID3DX11EffectVectorVariable, AsVector)

BEGIN_NS
class GMDx11EffectVariableBank
{
public:
	void init(ID3DX11Effect* effect)
	{
		m_effect = effect;
	}

	// Shadow
	EFFECT_VARIABLE(ShadowInfo, GM_VariablesDesc.ShadowInfo.ShadowInfo)
	EFFECT_MEMBER_AS_SCALAR(HasShadow, ShadowInfo(), GM_VariablesDesc.ShadowInfo.HasShadow)
	EFFECT_MEMBER_AS_MATRIX(ShadowMatrix, ShadowInfo(), GM_VariablesDesc.ShadowInfo.ShadowMatrix)
	EFFECT_MEMBER_AS_VECTOR(ShadowPosition, ShadowInfo(), GM_VariablesDesc.ShadowInfo.Position)
	EFFECT_MEMBER_AS_SCALAR(ShadowMapWidth, ShadowInfo(), GM_VariablesDesc.ShadowInfo.ShadowMapWidth)
	EFFECT_MEMBER_AS_SCALAR(ShadowMapHeight, ShadowInfo(), GM_VariablesDesc.ShadowInfo.ShadowMapHeight)
	EFFECT_MEMBER_AS_SCALAR(ShadowBiasMin, ShadowInfo(), GM_VariablesDesc.ShadowInfo.BiasMin)
	EFFECT_MEMBER_AS_SCALAR(ShadowBiasMax, ShadowInfo(), GM_VariablesDesc.ShadowInfo.BiasMax)
	EFFECT_VARIABLE_AS_SHADER_RESOURCE(ShadowMap, GM_VariablesDesc.ShadowInfo.ShadowMap)
	EFFECT_VARIABLE_AS_SHADER_RESOURCE(ShadowMapMSAA, GM_VariablesDesc.ShadowInfo.ShadowMapMSAA)

	// ScreenInfo
	EFFECT_VARIABLE(ScreenInfo, GM_VariablesDesc.ScreenInfoAttributes.ScreenInfo)
	EFFECT_MEMBER_AS_SCALAR(ScreenWidth, ScreenInfo(), GM_VariablesDesc.ScreenInfoAttributes.ScreenWidth)
	EFFECT_MEMBER_AS_SCALAR(ScreenHeight, ScreenInfo(), GM_VariablesDesc.ScreenInfoAttributes.ScreenHeight)
	EFFECT_MEMBER_AS_SCALAR(ScreenMultiSampling, ScreenInfo(), GM_VariablesDesc.ScreenInfoAttributes.Multisampling)

	// Material
	EFFECT_VARIABLE(Material, GM_VariablesDesc.MaterialName)
	EFFECT_MEMBER_AS_VECTOR(Ka, Material(), GM_VariablesDesc.MaterialAttributes.Ka)
	EFFECT_MEMBER_AS_VECTOR(Kd, Material(), GM_VariablesDesc.MaterialAttributes.Kd)
	EFFECT_MEMBER_AS_VECTOR(Ks, Material(), GM_VariablesDesc.MaterialAttributes.Ks)
	EFFECT_MEMBER_AS_SCALAR(Shininess, Material(), GM_VariablesDesc.MaterialAttributes.Shininess)
	EFFECT_MEMBER_AS_SCALAR(Refreactivity, Material(), GM_VariablesDesc.MaterialAttributes.Refreactivity)
	EFFECT_MEMBER_AS_VECTOR(F0, Material(), GM_VariablesDesc.MaterialAttributes.F0)

	// Filter
	EFFECT_VARIABLE_AS_SCALAR(KernelDeltaX, GM_VariablesDesc.FilterAttributes.KernelDeltaX)
	EFFECT_VARIABLE_AS_SCALAR(KernelDeltaY, GM_VariablesDesc.FilterAttributes.KernelDeltaY)

private:
	ID3DX11Effect* m_effect = nullptr;
};
END_NS

GMDx11EffectVariableBank& GMDx11Renderer::getVarBank()
{
	D(d);
	if (!d->bank)
		d->bank = new GMDx11EffectVariableBank();
	return *d->bank;
}

ITexture* GMDx11Renderer::getWhiteTexture()
{
	D(d);
	D_BASE(db, Base);
	if (!d->whiteTexture)
		d->whiteTexture = createWhiteTexture(d->context);
	return d->whiteTexture;
}

BEGIN_NS
struct GMDx11RasterizerStates
{
	enum
	{
		Size_Cull = 2,
		Size_FrontFace = 2,
	};

	//TODO 先不考虑FillMode
public:
	GMDx11RasterizerStates::GMDx11RasterizerStates(const IRenderContext* context)
	{
		this->context = context;
		this->engine = gm_cast<GMDx11GraphicEngine*>(context->getEngine());
	}

public:
	ID3D11RasterizerState* getRasterStates(GMS_FrontFace frontFace, GMS_Cull cullMode)
	{
		const GMWindowStates& windowStates = context->getWindow()->getWindowStates();
		bool multisampleEnable = windowStates.sampleCount > 1;
		GMComPtr<ID3D11RasterizerState>& state = states[(GMuint)cullMode][(GMuint)frontFace];
		if (!state)
		{
			D3D11_RASTERIZER_DESC desc = getRasterizerDesc(frontFace, cullMode, multisampleEnable, multisampleEnable);
			createRasterizerState(desc, &state);
		}

		GM_ASSERT(state);
		return state;
	}

private:
	bool createRasterizerState(const D3D11_RASTERIZER_DESC& desc, ID3D11RasterizerState** out)
	{
		GM_DX_HR(engine->getDevice()->CreateRasterizerState(&desc, out));
		return !!(*out);
	}

private:
	const IRenderContext* context = nullptr;
	GMDx11GraphicEngine* engine = nullptr;
	GMComPtr<ID3D11RasterizerState> states[Size_Cull][Size_FrontFace];
};

struct GMDx11BlendStates
{
public:
	GMDx11BlendStates::GMDx11BlendStates(const IRenderContext* context)
	{
		this->context = context;
		this->engine = gm_cast<GMDx11GraphicEngine*>(context->getEngine());
	}

public:
	ID3D11BlendState* getBlendState(
		bool enable,
		GMS_BlendFunc srcRGB,
		GMS_BlendFunc destRGB,
		GMS_BlendOp opRGB,
		GMS_BlendFunc srcAlpha,
		GMS_BlendFunc destAlpha,
		GMS_BlendOp opAlpha
	)
	{
		GMComPtr<ID3D11BlendState>& state = states[enable ? 1 : 0][(GMuint)srcRGB][(GMuint)destRGB][(GMuint)opRGB][(GMuint)srcAlpha][(GMuint)destAlpha][(GMuint)opAlpha];
		if (!state)
		{
			D3D11_BLEND_DESC desc = getBlendDesc(enable, srcRGB, destRGB, opRGB, srcAlpha, destAlpha, opAlpha);
			createBlendState(desc, &state);
		}

		GM_ASSERT(state);
		return state;
	}

	ID3D11BlendState* getDisabledBlendState()
	{
		return getBlendState(
			false,
			GMS_BlendFunc::ONE,
			GMS_BlendFunc::ONE,
			GMS_BlendOp::ADD,
			GMS_BlendFunc::ONE,
			GMS_BlendFunc::ONE,
			GMS_BlendOp::ADD
		);
	}

private:
	bool createBlendState(const D3D11_BLEND_DESC& desc, ID3D11BlendState** out)
	{
		GM_DX_HR(engine->getDevice()->CreateBlendState(&desc, out));
		return !!(*out);
	}

private:
	const IRenderContext* context = nullptr;
	GMDx11GraphicEngine* engine = nullptr;
	GMComPtr<ID3D11BlendState> states[2]
		[(GMuint)GMS_BlendFunc::MAX_OF_BLEND_FUNC]	// Source RGB
		[(GMuint)GMS_BlendFunc::MAX_OF_BLEND_FUNC]	// Dest RGB
		[(GMuint)GMS_BlendOp::MAX_OF_BLEND_OP]		// Op RGB
		[(GMuint)GMS_BlendFunc::MAX_OF_BLEND_FUNC]	// Source Alpha
		[(GMuint)GMS_BlendFunc::MAX_OF_BLEND_FUNC]	// Dest Alpha
		[(GMuint)GMS_BlendOp::MAX_OF_BLEND_OP]		// Op Alpha
	;
};

struct GMDx11DepthStencilStates
{
public:
	GMDx11DepthStencilStates::GMDx11DepthStencilStates(const IRenderContext* context)
	{
		this->context = context;
		this->engine = gm_cast<GMDx11GraphicEngine*>(context->getEngine());
	}

public:
	ID3D11DepthStencilState* getDepthStencilState(bool depthEnabled, const GMStencilOptions& stencilOptions)
	{
		GMComPtr<ID3D11DepthStencilState>& state = states[depthEnabled ? 1 : 0][stencilOptions.writeMask == GMStencilOptions::Ox00 ? 1 : 0][stencilOptions.compareOp];
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
	const IRenderContext* context = nullptr;
	GMDx11GraphicEngine* engine = nullptr;
	GMComPtr<ID3D11DepthStencilState> states[2][2][3];
};

END_NS

GMDx11Renderer::GMDx11Renderer(const IRenderContext* context)
{
	D(d);
	d->context = context;

	IShaderProgram* shaderProgram = getEngine()->getShaderProgram();
	shaderProgram->useProgram();
	GM_ASSERT(!d->effect);
	shaderProgram->getInterface(GameMachineInterfaceID::D3D11Effect, (void**)&d->effect);
	GM_ASSERT(d->effect);

	d->deviceContext = getEngine()->getDeviceContext();
	getVarBank().init(d->effect);
}

GMDx11Renderer::~GMDx11Renderer()
{
	D(d);
	GM_delete(d->whiteTexture);
	GM_delete(d->rasterizerStates);
	GM_delete(d->blendStates);
	GM_delete(d->depthStencilStates);
	GM_delete(d->bank);
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
	
	if (parent)
	{
		shaderProgram->setMatrix4(GM_VariablesDesc.ModelMatrix, parent->getTransform());
		shaderProgram->setMatrix4(GM_VariablesDesc.InverseTransposeModelMatrix, InverseTranspose(parent->getTransform()));
	}
	else
	{
		shaderProgram->setMatrix4(GM_VariablesDesc.ModelMatrix, Identity<GMMat4>());
		shaderProgram->setMatrix4(GM_VariablesDesc.InverseTransposeModelMatrix, Identity<GMMat4>());
	}

	GMCamera& camera = d->engine->getCamera();
	if (camera.isDirty())
	{
		GMFloat4 viewPosition;
		camera.getLookAt().position.loadFloat4(viewPosition);
		shaderProgram->setVec4(GM_VariablesDesc.ViewPosition, viewPosition);
		shaderProgram->setMatrix4(GM_VariablesDesc.ViewMatrix, camera.getViewMatrix());
		shaderProgram->setMatrix4(GM_VariablesDesc.ProjectionMatrix, camera.getProjectionMatrix());
		shaderProgram->setMatrix4(GM_VariablesDesc.InverseViewMatrix, camera.getInverseViewMatrix());
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

const IRenderContext* GMDx11Renderer::getContext()
{
	D(d);
	return d->context;
}

void GMDx11Renderer::prepareScreenInfo()
{
	D(d);
	// 如果屏幕更改了，需要通知此处重新设置着色器
	if (!d->screenInfoPrepared)
	{
		GMDx11EffectVariableBank& bank = getVarBank();
		const GMWindowStates& windowStates = getContext()->getWindow()->getWindowStates();
		ID3DX11EffectVariable* screenInfo = d->effect->GetVariableByName(GM_VariablesDesc.ScreenInfoAttributes.ScreenInfo);
		GM_ASSERT(screenInfo->IsValid());

		ID3DX11EffectScalarVariable* screenWidth = bank.ScreenWidth();
		GM_DX_HR(screenWidth->SetInt(windowStates.renderRect.width));

		ID3DX11EffectScalarVariable* screenHeight = bank.ScreenHeight();
		GM_DX_HR(screenHeight->SetInt(windowStates.renderRect.height));

		ID3DX11EffectScalarVariable* multisampling = bank.ScreenMultiSampling();
		GM_DX_HR(multisampling->SetBool(windowStates.sampleCount > 1));
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
		else
		{
			if (model->getShader().getIlluminationModel() == GMIlluminationModel::Phong && (
				type == GMTextureType::Ambient ||
				type == GMTextureType::Diffuse ||
				type == GMTextureType::Specular ||
				type == GMTextureType::Lightmap
				))
			{
				ITexture* whiteTexture = getWhiteTexture();
				applyTextureAttribute(model, whiteTexture, type);
				whiteTexture->bindSampler(nullptr);
				whiteTexture->useTexture((GMint)type);
			}
		}
	}

	const GMDx11CubeMapState& cubeMapState = getEngine()->getCubeMapState();
	if (cubeMapState.hasCubeMap)
	{
		GM_ASSERT(cubeMapState.model && cubeMapState.cubeMapRenderer);
		cubeMapState.cubeMapRenderer->prepareTextures(cubeMapState.model);
	}
}

void GMDx11Renderer::applyTextureAttribute(GMModel* model, ITexture* texture, GMTextureType type)
{
	D(d);
	const char* textureName = nullptr;
	switch (type)
	{
	case GMTextureType::Ambient:
		textureName = GM_VariablesDesc.AmbientTextureName;
		break;
	case GMTextureType::Diffuse:
		textureName = GM_VariablesDesc.DiffuseTextureName;
		break;
	case GMTextureType::Specular:
		textureName = GM_VariablesDesc.SpecularTextureName;
		break;
	case GMTextureType::NormalMap:
		textureName = GM_VariablesDesc.NormalMapTextureName;
		break;
	case GMTextureType::Lightmap:
		textureName = GM_VariablesDesc.LightMapTextureName;
		break;
	case GMTextureType::CubeMap:
		textureName = GM_VariablesDesc.CubeMapTextureName;
		break;
	case GMTextureType::Albedo:
		textureName = GM_VariablesDesc.AlbedoTextureName;
		break;
	case GMTextureType::MetallicRoughnessAO:
		textureName = GM_VariablesDesc.MetallicRoughnessAOTextureName;
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
			newBank.enabled = textureAttribute->GetMemberByName(GM_VariablesDesc.TextureAttributes.Enabled)->AsScalar();
			newBank.offsetX = textureAttribute->GetMemberByName(GM_VariablesDesc.TextureAttributes.OffsetX)->AsScalar();
			newBank.offsetY = textureAttribute->GetMemberByName(GM_VariablesDesc.TextureAttributes.OffsetY)->AsScalar();
			newBank.scaleX = textureAttribute->GetMemberByName(GM_VariablesDesc.TextureAttributes.ScaleX)->AsScalar();
			newBank.scaleY = textureAttribute->GetMemberByName(GM_VariablesDesc.TextureAttributes.ScaleY)->AsScalar();
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
		GM_DX_HR(bank->offsetX->SetFloat(0.f));
		GM_DX_HR(bank->offsetY->SetFloat(0.f));
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

		model->getShader().getTextureList().getTextureSampler(type).applyTexMode(GM.getGameMachineRunningStates().elapsedTime, applyCallback);
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
	const GMWindowStates& windowStates = d->context->getWindow()->getWindowStates();
	bool multisampleEnable = windowStates.sampleCount > 1;
	if (!d->rasterizer)
	{
		d->rasterizer = d->effect->GetVariableByName(GM_VariablesDesc.RasterizerState)->AsRasterizer();
	}

	if (!d->rasterizerStates)
		d->rasterizerStates = new GMDx11RasterizerStates(getContext());

	GM_ASSERT(d->rasterizer);
	GM_DX_HR(d->rasterizer->SetRasterizerState(
		0, 
		d->rasterizerStates->getRasterStates(model->getShader().getFrontFace(), model->getShader().getCull())
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
	GM_DX_HR(bank.F0()->SetFloatVector(ValuePointer(material.f0)));

	IShaderProgram* shaderProgram = getEngine()->getShaderProgram();
	GMIlluminationModel illuminationModel = shader.getIlluminationModel();
	shaderProgram->setInt(GM_VariablesDesc.IlluminationModel, (GMint)illuminationModel);
}

void GMDx11Renderer::prepareBlend(GMModel* model)
{
	D(d);
	if (!d->blend)
	{
		d->blend = d->effect->GetVariableByName(GM_VariablesDesc.BlendState)->AsBlend();
	}
	GM_ASSERT(d->blend);

	const GMGlobalBlendStateDesc& globalBlendState = getEngine()->getGlobalBlendState();
	if (!d->blendStates)
		d->blendStates = new GMDx11BlendStates(getContext());

	const GMShader& shader = model->getShader();
	if (globalBlendState.enabled)
	{
		// 全局blend开启时
		if (model->getShader().getBlend())
		{
			GM_DX_HR(d->blend->SetBlendState(
				0,
				d->blendStates->getBlendState(
					true, 
					shader.getBlendFactorSourceRGB(),
					shader.getBlendFactorDestRGB(),
					shader.getBlendOpRGB(),
					shader.getBlendFactorSourceAlpha(),
					shader.getBlendFactorDestAlpha(),
					shader.getBlendOpAlpha()
				)
			));
		}
		else
		{
			GM_DX_HR(d->blend->SetBlendState(
				0,
				d->blendStates->getBlendState(
					true,
					globalBlendState.sourceRGB,
					globalBlendState.destRGB,
					globalBlendState.opRGB,
					globalBlendState.sourceAlpha,
					globalBlendState.destAlpha,
					globalBlendState.opAlpha
				)
			));
		}
	}
	else
	{
		// 全局blend关闭，此时应该应用正在绘制物体的Blend状态
		if (shader.getBlend())
		{
			GM_DX_HR(d->blend->SetBlendState(
				0,
				d->blendStates->getBlendState(
					true,
					shader.getBlendFactorSourceRGB(),
					shader.getBlendFactorDestRGB(),
					shader.getBlendOpRGB(),
					shader.getBlendFactorSourceAlpha(),
					shader.getBlendFactorDestAlpha(),
					shader.getBlendOpAlpha()
				)
			));
		}
		else
		{
			GM_DX_HR(d->blend->SetBlendState(
				0,
				d->blendStates->getDisabledBlendState()
			));
		}
	}
}

void GMDx11Renderer::prepareDepthStencil(GMModel* model)
{
	D(d);
	if (!d->depthStencil)
	{
		d->depthStencil = d->effect->GetVariableByName(GM_VariablesDesc.DepthStencilState)->AsDepthStencil();
	}
	GM_ASSERT(d->depthStencil);

	if (!d->depthStencilStates)
		d->depthStencilStates = new GMDx11DepthStencilStates(getContext());

	GM_DX_HR(d->depthStencil->SetDepthStencilState(
		0,
		d->depthStencilStates->getDepthStencilState(
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
	GMint elapsed = GM.getGameMachineRunningStates().elapsedTime * 1000;

	return sampler.getFrameByIndex((elapsed / sampler.getAnimationMs()) % sampler.getFrameCount());
}

void GMDx11Renderer::setGamma(IShaderProgram* shaderProgram)
{
	D(d);
	bool needGammaCorrection = getEngine()->needGammaCorrection();
	shaderProgram->setBool(GM_VariablesDesc.GammaCorrection.GammaCorrection, needGammaCorrection);
	GMfloat gamma = getEngine()->getGammaValue();
	if (gamma != d->gamma)
	{
		shaderProgram->setFloat(GM_VariablesDesc.GammaCorrection.GammaValue, gamma);
		shaderProgram->setFloat(GM_VariablesDesc.GammaCorrection.GammaInvValue, 1.f / gamma);
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
		GMDx11CubeMapState& cubeMapState = getEngine()->getCubeMapState();
		if (cubeMapState.model != model)
		{
			cubeMapState.hasCubeMap = true;
			cubeMapState.cubeMapRenderer = this;
			cubeMapState.model = model;
		}
	}
}

GMDx11Renderer_Filter::GMDx11Renderer_Filter(const IRenderContext* context)
	: GMDx11Renderer(context)
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
	const GMWindowStates& windowStates = d->context->getWindow()->getWindowStates();
	D3DX11_TECHNIQUE_DESC techDesc;
	GM_DX_HR(getTechnique()->GetDesc(&techDesc));

	for (GMuint p = 0; p < techDesc.Passes; ++p)
	{
		GMDx11Texture* filterTexture = gm_cast<GMDx11Texture*>(model->getShader().getTextureList().getTextureSampler(GMTextureType::Ambient).getFrameByIndex(0));
		GM_ASSERT(filterTexture);
		ID3DX11EffectPass* pass = getTechnique()->GetPassByIndex(p);
		if (windowStates.sampleCount == 1)
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
	bool b = shaderProgram->setInterfaceInstance(GM_VariablesDesc.FilterAttributes.Filter, GM_VariablesDesc.FilterAttributes.Types[filterMode], GMShaderType::Effect);
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
			shaderProgram->setBool(GM_VariablesDesc.HDR.HDR, false);
		}
	}
}

void GMDx11Renderer_Filter::setHDR(IShaderProgram* shaderProgram)
{
	D(d);
	shaderProgram->setBool(GM_VariablesDesc.HDR.HDR, true);
	if (d->state.toneMapping == GMToneMapping::Reinhard)
	{
		bool b = shaderProgram->setInterfaceInstance(GM_VariablesDesc.HDR.ToneMapping, "ReinhardToneMapping", GMShaderType::Effect);
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

void GMDx11Renderer_Deferred_3D_LightPass::prepareTextures(GMModel* model)
{
	D(d);
	GMDx11GBuffer* gbuffer = gm_cast<GMDx11GBuffer*>(getEngine()->getGBuffer());
	gbuffer->useGeometryTextures(d->effect);

	const GMDx11CubeMapState& cubeMapState = getEngine()->getCubeMapState();
	if (cubeMapState.hasCubeMap)
	{
		GM_ASSERT(cubeMapState.model && cubeMapState.cubeMapRenderer);
		cubeMapState.cubeMapRenderer->prepareTextures(cubeMapState.model);
	}
}

void GMDx11Renderer_3D_Shadow::beginModel(GMModel* model, const GMGameObject* parent)
{
	D(d);
	const GMWindowStates& windowStates = d->context->getWindow()->getWindowStates();
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

	if (parent)
	{
		shaderProgram->setMatrix4(GM_VariablesDesc.ModelMatrix, parent->getTransform());
		shaderProgram->setMatrix4(GM_VariablesDesc.InverseTransposeModelMatrix, InverseTranspose(parent->getTransform()));
	}
	else
	{
		shaderProgram->setMatrix4(GM_VariablesDesc.ModelMatrix, Identity<GMMat4>());
		shaderProgram->setMatrix4(GM_VariablesDesc.InverseTransposeModelMatrix, Identity<GMMat4>());
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

	ID3DX11EffectShaderResourceVariable* shadowMap = windowStates.sampleCount > 1 ? bank.ShadowMapMSAA() : bank.ShadowMap();
	GM_DX_HR(shadowMap->SetResource(shadowFramebuffers->getShadowMapShaderResourceView()));
}