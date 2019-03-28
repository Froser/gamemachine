#include "stdafx.h"
#include "gmdx11techniques.h"
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
#define GMSHADER_SEMANTIC_NAME_BONES "BONES"
#define GMSHADER_SEMANTIC_NAME_WEIGHTS "WEIGHTS"
#define BIT32_OFFSET(i) (sizeof(gm::GMfloat) * i)
#define CHECK_VAR(var) if (!var->IsValid()) { return; }

#define getVariableIndex(shaderProgram, index, name) (index ? index : (index = shaderProgram->getIndex(name)))
#define VI_SP(d, name, shaderProgram, indexBank) \
	getVariableIndex(shaderProgram, d->indexBank. name, GM_VariablesDesc. ## name)
#define VI(name) VI_SP(d, name, shaderProgram, indexBank)
#define VI_B(name) VI_SP(db, name, shaderProgram, indexBank)

namespace
{
	D3D11_INPUT_ELEMENT_DESC GMSHADER_ElementDescriptions[] =
	{
		{ GMSHADER_SEMANTIC_NAME_POSITION, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, BIT32_OFFSET(0), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 3

		{ GMSHADER_SEMANTIC_NAME_NORMAL, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, BIT32_OFFSET(3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 3

		{ GMSHADER_SEMANTIC_NAME_TEXCOORD, 0, DXGI_FORMAT_R32G32_FLOAT, 0, BIT32_OFFSET(6), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 2

		{ GMSHADER_SEMANTIC_NAME_TANGENT, 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, BIT32_OFFSET(8), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 3

		{ GMSHADER_SEMANTIC_NAME_BITANGENT, 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, BIT32_OFFSET(11), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 3

		{ GMSHADER_SEMANTIC_NAME_LIGHTMAP, 1, DXGI_FORMAT_R32G32_FLOAT, 0, BIT32_OFFSET(14), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 2

		{ GMSHADER_SEMANTIC_NAME_COLOR, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, BIT32_OFFSET(16), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 4

		{ GMSHADER_SEMANTIC_NAME_BONES, 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, BIT32_OFFSET(20), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 4

		{ GMSHADER_SEMANTIC_NAME_WEIGHTS, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, BIT32_OFFSET(24), D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
			return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		default:
			GM_ASSERT(false);
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
	}

	inline D3D11_RASTERIZER_DESC getRasterizerDesc(
		bool isSolid,
		GMS_FrontFace frontFace,
		GMS_Cull cull,
		bool multisampleEnable,
		bool antialiasedLineEnable
	)
	{
		D3D11_RASTERIZER_DESC desc = {
			isSolid ? D3D11_FILL_SOLID : D3D11_FILL_WIREFRAME,
			cull == GMS_Cull::Cull ? D3D11_CULL_BACK : D3D11_CULL_NONE,
			frontFace == GMS_FrontFace::Closewise ? FALSE : TRUE,
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
		case (GMS_BlendFunc::Zero):
			return D3D11_BLEND_ZERO;
		case (GMS_BlendFunc::One):
			return D3D11_BLEND_ONE;
		case (GMS_BlendFunc::SourceColor):
			return D3D11_BLEND_SRC_COLOR;
		case (GMS_BlendFunc::DestColor):
			return D3D11_BLEND_DEST_COLOR;
		case (GMS_BlendFunc::SourceAlpha):
			return D3D11_BLEND_SRC_ALPHA;
		case (GMS_BlendFunc::DestAlpha):
			return D3D11_BLEND_DEST_ALPHA;
		case (GMS_BlendFunc::OneMinusSourceAlpha):
			return D3D11_BLEND_INV_SRC_ALPHA;
		case (GMS_BlendFunc::OneMinusDestAlpha):
			return D3D11_BLEND_INV_DEST_ALPHA;
		case (GMS_BlendFunc::OneMinusSourceColor):
			return D3D11_BLEND_INV_SRC_COLOR;
		case (GMS_BlendFunc::OneMinusDestColor):
			return D3D11_BLEND_INV_DEST_COLOR;
		default:
			GM_ASSERT(false);
			return D3D11_BLEND_ONE;
		}
	}

	inline D3D11_BLEND_OP toDx11BlendOp(GMS_BlendOp op)
	{
		D3D11_BLEND_OP blendOp = D3D11_BLEND_OP_ADD;
		if (op == GMS_BlendOp::Add)
		{
			blendOp = D3D11_BLEND_OP_ADD;
		}
		else if (op == GMS_BlendOp::Substract)
		{
			blendOp = D3D11_BLEND_OP_SUBTRACT;
		}
		else if (op == GMS_BlendOp::ReverseSubstract)
		{
			blendOp = D3D11_BLEND_OP_REV_SUBTRACT;
		}
		else
		{
			GM_ASSERT(false);
			gm_error(gm_dbg_wrap("Invalid blend op"));
			blendOp = D3D11_BLEND_OP_ADD;
		}
		return blendOp;
	}

	inline D3D11_STENCIL_OP toStencilOp(GMStencilOptions::GMStencilOp stencilOptions)
	{
		switch (stencilOptions)
		{
		case GMStencilOptions::Keep:
			return D3D11_STENCIL_OP_KEEP;
		case GMStencilOptions::Zero:
			return D3D11_STENCIL_OP_ZERO;
		case GMStencilOptions::Replace:
			return D3D11_STENCIL_OP_REPLACE;
		default:
			GM_ASSERT(false);
			return D3D11_STENCIL_OP_KEEP;
		}
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
		for (GMuint32 i = 0; i < 8; ++i)
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

		desc.FrontFace.StencilFailOp = desc.BackFace.StencilFailOp = toStencilOp(stencilOptions.stencilFailedOp);
		desc.FrontFace.StencilDepthFailOp = desc.BackFace.StencilDepthFailOp = toStencilOp(stencilOptions.stencilDepthFailedOp);
		desc.FrontFace.StencilPassOp = desc.BackFace.StencilPassOp = toStencilOp(stencilOptions.stencilPassOp);
		desc.FrontFace.StencilFunc = desc.BackFace.StencilFunc = (
			(stencilOptions.compareFunc == GMStencilOptions::Equal) ? D3D11_COMPARISON_EQUAL :
			(stencilOptions.compareFunc == GMStencilOptions::NotEqual) ? D3D11_COMPARISON_NOT_EQUAL : 
			(stencilOptions.compareFunc == GMStencilOptions::Less) ? D3D11_COMPARISON_LESS :
			(stencilOptions.compareFunc == GMStencilOptions::LessEqual) ? D3D11_COMPARISON_LESS_EQUAL :
			(stencilOptions.compareFunc == GMStencilOptions::Greater) ? D3D11_COMPARISON_GREATER :
			(stencilOptions.compareFunc == GMStencilOptions::GreaterEqual) ? D3D11_COMPARISON_GREATER_EQUAL :
			(stencilOptions.compareFunc == GMStencilOptions::Never) ? D3D11_COMPARISON_NEVER : D3D11_COMPARISON_ALWAYS
		);
		return desc;
	}

	GMTextureAsset createWhiteTexture(const IRenderContext* context)
	{
		GMTextureAsset texture;
		GM.getFactory()->createWhiteTexture(context, texture);
		return texture;
	}
}

#define EFFECT_VARIABLE(funcName, name) \
	private: ID3DX11EffectVariable* effect_var_##funcName = nullptr;										\
	public: ID3DX11EffectVariable* funcName() {																\
		ID3DX11EffectVariable* effect_var_##funcName = nullptr;												\
		if (!effect_var_##funcName) {																		\
			effect_var_##funcName = m_effect->GetVariableByName(name.toStdString().c_str());}				\
		return effect_var_##funcName;																		\
	}

#define EFFECT_VARIABLE_AS(funcName, name, retType, to) \
	private: retType* effect_var_##funcName = nullptr;														\
	public: retType* funcName() {																			\
		retType* effect_var_##funcName = nullptr;															\
		if (!effect_var_##funcName) {																		\
			effect_var_##funcName = m_effect->GetVariableByName(name.toStdString().c_str())->to(); }		\
		return effect_var_##funcName;																		\
	}

#define EFFECT_VARIABLE_AS_SHADER_RESOURCE(funcName, name) \
	EFFECT_VARIABLE_AS(funcName, name, ID3DX11EffectShaderResourceVariable, AsShaderResource)

#define EFFECT_VARIABLE_AS_SCALAR(funcName, name) \
	EFFECT_VARIABLE_AS(funcName, name, ID3DX11EffectScalarVariable, AsScalar)

#define EFFECT_MEMBER_AS(funcName, effect, name, retType, to) \
	private: retType* effect_var_##funcName = nullptr;														\
	public: retType* funcName() {																			\
		retType* effect_var_##funcName = nullptr;															\
		if (!effect_var_##funcName) {																		\
			effect_var_##funcName = effect->GetMemberByName(name.toStdString().c_str())->to();}				\
		return effect_var_##funcName;																		\
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
	EFFECT_MEMBER_AS_SCALAR(EndClip, ShadowInfo(), GM_VariablesDesc.ShadowInfo.EndClip)
	EFFECT_MEMBER_AS_SCALAR(CurrentCascadeLevel, ShadowInfo(), GM_VariablesDesc.ShadowInfo.CurrentCascadeLevel)
	EFFECT_MEMBER_AS_VECTOR(ShadowPosition, ShadowInfo(), GM_VariablesDesc.ShadowInfo.Position)
	EFFECT_MEMBER_AS_SCALAR(ShadowMapWidth, ShadowInfo(), GM_VariablesDesc.ShadowInfo.ShadowMapWidth)
	EFFECT_MEMBER_AS_SCALAR(ShadowMapHeight, ShadowInfo(), GM_VariablesDesc.ShadowInfo.ShadowMapHeight)
	EFFECT_MEMBER_AS_SCALAR(ShadowBiasMin, ShadowInfo(), GM_VariablesDesc.ShadowInfo.BiasMin)
	EFFECT_MEMBER_AS_SCALAR(ShadowBiasMax, ShadowInfo(), GM_VariablesDesc.ShadowInfo.BiasMax)
	EFFECT_MEMBER_AS_SCALAR(CascadedShadowLevel, ShadowInfo(), GM_VariablesDesc.ShadowInfo.CascadedShadowLevel)
	EFFECT_MEMBER_AS_SCALAR(ViewCascade, ShadowInfo(), GM_VariablesDesc.ShadowInfo.ViewCascade)
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

	// Textures
	EFFECT_VARIABLE(AmbientTexture, GM_VariablesDesc.AmbientTextureName);
	EFFECT_VARIABLE(DiffuseTexture, GM_VariablesDesc.DiffuseTextureName);
	EFFECT_VARIABLE(SpecularTexture, GM_VariablesDesc.SpecularTextureName);
	EFFECT_VARIABLE(NormalMapTexture, GM_VariablesDesc.NormalMapTextureName);
	EFFECT_VARIABLE(LightMapTexture, GM_VariablesDesc.LightMapTextureName);
	EFFECT_VARIABLE(CubeMapTexture, GM_VariablesDesc.CubeMapTextureName);
	EFFECT_VARIABLE(AlbedoTexture, GM_VariablesDesc.AlbedoTextureName);
	EFFECT_VARIABLE(MetallicRoughnessAOTexture, GM_VariablesDesc.MetallicRoughnessAOTextureName);

private:
	ID3DX11Effect* m_effect = nullptr;
};
END_NS

GMDx11EffectVariableBank& GMDx11Technique::getVarBank()
{
	D(d);
	if (!d->bank)
		d->bank = gm_makeOwnedPtr<GMDx11EffectVariableBank>();
	return *d->bank;
}

GMTextureAsset GMDx11Technique::getWhiteTexture()
{
	D(d);
	D_BASE(db, Base);
	if (d->whiteTexture.isEmpty())
		d->whiteTexture = createWhiteTexture(d->context);
	return d->whiteTexture;
}

void GMDx11Technique::updateBoneTransforms(IShaderProgram* shaderProgram, GMModel* model)
{
	D(d);
	auto bones = d->effect->GetVariableByName(GM_VariablesDesc.Bones.toStdString().c_str());
	const auto& transforms = model->getBoneTransformations();
	for (UINT i = 0; i < gm_sizet_to_uint(transforms.size()); ++i)
	{
		const auto& transform = transforms[i];
		bones->GetElement(i)->AsMatrix()->SetMatrix(ValuePointer(transform));
	}
}

void GMDx11Technique::setCascadeEndClip(GMCascadeLevel level, GMfloat endClip)
{
	D(d);
	GMDx11EffectVariableBank& bank = getVarBank();
	GM_DX_HR(bank.EndClip()->GetElement(level)->AsScalar()->SetFloat(endClip));
}

void GMDx11Technique::setCascadeCameraVPMatrices(GMCascadeLevel level)
{
	D(d);
	GMDx11EffectVariableBank& bank = getVarBank();
	ID3DX11EffectVariable* shadowMatrix = bank.ShadowMatrix();
	GM_DX_HR(shadowMatrix->GetElement(level)->AsMatrix()->SetMatrix(ValuePointer(getEngine()->getCascadeCameraVPMatrix(level))));
}

const std::string& GMDx11Technique::getTechniqueNameByTechniqueId(GMRenderTechinqueID id)
{
	static Vector<std::string> s_names;

	GM_ASSERT(id - GMRenderTechniqueManager::StartupTechinqueID - 1 >= 0);
	GMsize_t index = id - GMRenderTechniqueManager::StartupTechinqueID - 1;
	if (index < s_names.size())
		return s_names[index];

	std::string* result = nullptr;
	GMMutex s_mutex;
	s_mutex.lock();
	s_names.resize(index + 1);
	s_names[index] = "GMTech_Custom" + GMString(id).toStdString();
	result = &s_names[index];
	s_mutex.unlock();
	return *result;
}

BEGIN_NS
struct GMDx11RasterizerStates
{
	enum
	{
		Size_FillMode = 2,
		Size_Cull = 2,
		Size_FrontFace = 2,
	};

	enum
	{
		WireFrame,
		Solid,
	};

public:
	GMDx11RasterizerStates::GMDx11RasterizerStates(const IRenderContext* context)
	{
		this->context = context;
		this->engine = gm_cast<GMDx11GraphicEngine*>(context->getEngine());
	}

public:
	ID3D11RasterizerState* getRasterStates(GMModel* model)
	{
		const GMWindowStates& windowStates = context->getWindow()->getWindowStates();
		bool multisampleEnable = windowStates.sampleCount > 1;

		GMS_FrontFace frontFace = model->getShader().getFrontFace();
		GMS_Cull cullMode = model->getShader().getCull();
		GMuint32 fillMode = engine->isWireFrameMode(model) ? WireFrame : Solid;

		GMComPtr<ID3D11RasterizerState>& state = states[fillMode][(GMuint32)cullMode][(GMuint32)frontFace];
		if (!state)
		{
			D3D11_RASTERIZER_DESC desc = getRasterizerDesc(fillMode != WireFrame, frontFace, cullMode, multisampleEnable, multisampleEnable);
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
	GMComPtr<ID3D11RasterizerState> states[Size_FillMode][Size_Cull][Size_FrontFace];
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
		GMComPtr<ID3D11BlendState>& state = states[enable ? 1 : 0][(GMuint32)srcRGB][(GMuint32)destRGB][(GMuint32)opRGB][(GMuint32)srcAlpha][(GMuint32)destAlpha][(GMuint32)opAlpha];
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
			GMS_BlendFunc::One,
			GMS_BlendFunc::One,
			GMS_BlendOp::Add,
			GMS_BlendFunc::One,
			GMS_BlendFunc::One,
			GMS_BlendOp::Add
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
		[(GMuint32)GMS_BlendFunc::MaxOfBlendFunc]	// Source RGB
		[(GMuint32)GMS_BlendFunc::MaxOfBlendFunc]	// Dest RGB
		[(GMuint32)GMS_BlendOp::MaxOfBlendOp]		// Op RGB
		[(GMuint32)GMS_BlendFunc::MaxOfBlendFunc]	// Source Alpha
		[(GMuint32)GMS_BlendFunc::MaxOfBlendFunc]	// Dest Alpha
		[(GMuint32)GMS_BlendOp::MaxOfBlendOp]		// Op Alpha
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
	GMComPtr<ID3D11DepthStencilState> getDepthStencilState(bool depthEnabled, const GMStencilOptions& stencilOptions)
	{
		GMComPtr<ID3D11DepthStencilState> state;
		D3D11_DEPTH_STENCIL_DESC desc = getDepthStencilDesc(depthEnabled, stencilOptions);
		createDepthStencilState(desc, &state);

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
};

END_NS

GMDx11Technique::GMDx11Technique(const IRenderContext* context)
{
	D(d);
	if (context)
	{
		d->context = context;

		IShaderProgram* shaderProgram = getEngine()->getShaderProgram();
		shaderProgram->useProgram();
		GM_ASSERT(!d->effect);
		shaderProgram->getInterface(GameMachineInterfaceID::D3D11Effect, (void**)&d->effect);
		GM_ASSERT(d->effect);

		d->deviceContext = getEngine()->getDeviceContext();
		d->debugConfig = GM.getConfigs().getConfig(GMConfigs::Debug).asDebugConfig();
		getVarBank().init(d->effect);
	}
}

void GMDx11Technique::beginScene(GMScene* scene)
{
	D(d);
	d->techContext.currentScene = scene;
	initShadow();
}

void GMDx11Technique::endScene()
{
	D(d);
	d->techContext.currentScene = nullptr;
}

void GMDx11Technique::beginModel(GMModel* model, const GMGameObject* parent)
{
	D(d);
	IShaderProgram* shaderProgram = getEngine()->getShaderProgram();
	shaderProgram->useProgram();
	d->techContext.currentModel = model;
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

	// Technique决定自己的顶点Layout
	d->deviceContext->IASetInputLayout(d->inputLayout);
	d->deviceContext->IASetPrimitiveTopology(getMode(model->getPrimitiveTopologyMode()));
	
	if (parent)
	{
		shaderProgram->setMatrix4(VI(ModelMatrix), parent->getTransform());
		shaderProgram->setMatrix4(VI(InverseTransposeModelMatrix), InverseTranspose(parent->getTransform()));
	}
	else
	{
		shaderProgram->setMatrix4(VI(ModelMatrix), Identity<GMMat4>());
		shaderProgram->setMatrix4(VI(InverseTransposeModelMatrix), Identity<GMMat4>());
	}

	GMCamera& camera = d->engine->getCamera();
	if (camera.isDirty())
	{
		GMFloat4 viewPosition;
		camera.getLookAt().position.loadFloat4(viewPosition);
		shaderProgram->setVec4(VI(ViewPosition), viewPosition);
		shaderProgram->setMatrix4(VI(ViewMatrix), camera.getViewMatrix());
		shaderProgram->setMatrix4(VI(ProjectionMatrix), camera.getProjectionMatrix());
		shaderProgram->setMatrix4(VI(InverseViewMatrix), camera.getInverseViewMatrix());
		camera.cleanDirty();
	}

	const GMShadowSourceDesc& shadowSourceDesc = getEngine()->getShadowSourceDesc();
	GMDx11EffectVariableBank& bank = getVarBank();
	ID3DX11EffectScalarVariable* hasShadow = bank.HasShadow();
	if (hasShadow->IsValid())
	{
		if (shadowSourceDesc.type != GMShadowSourceDesc::NoShadow)
		{
			GM_DX_HR(hasShadow->SetBool(true));

			ID3DX11EffectScalarVariable* cascadedShadowLevel = bank.CascadedShadowLevel();
			GM_DX_HR(cascadedShadowLevel->SetInt(shadowSourceDesc.cascades));
			prepareShadow(false);
		}
		else
		{
			GM_DX_HR(hasShadow->SetBool(false));
		}
	}

	setGamma(shaderProgram);

	// 设置顶点颜色运算方式
	shaderProgram->setInt(VI(ColorVertexOp), static_cast<GMint32>(model->getShader().getVertexColorOp()));

	// 骨骼动画
	GM_ASSERT(d->techContext.currentScene);
	if (d->techContext.currentScene->hasAnimation() && parent->isAnimationObject())
	{
		shaderProgram->setInt(VI(UseBoneAnimation), 1);
		updateBoneTransforms(shaderProgram, model);
	}
	else
	{
		shaderProgram->setInt(VI(UseBoneAnimation), 0);
	}
}

void GMDx11Technique::endModel()
{
	D(d);
	d->techContext.currentModel = nullptr;
}

const IRenderContext* GMDx11Technique::getContext()
{
	D(d);
	return d->context;
}

void GMDx11Technique::initShadow()
{
	D(d);
	// 如果有阴影，设置好阴影所有参数
	const GMShadowSourceDesc& shadowSourceDesc = getEngine()->getShadowSourceDesc();
	if (shadowSourceDesc.type != GMShadowSourceDesc::NoShadow)
	{
		if (d->engine->getGBuffer()->getGeometryPassingState() != GMGeometryPassingState::PassingGeometry)
		{
			ICSMFramebuffers* csm = getEngine()->getCSMFramebuffers();
			for (GMCascadeLevel i = 0; i < shadowSourceDesc.cascades; ++i)
			{
				setCascadeEndClip(i, csm->getEndClip(i));
				setCascadeCameraVPMatrices(i);
			}
		}
	}
}

void GMDx11Technique::prepareScreenInfo()
{
	D(d);
	// 如果屏幕更改了，需要通知此处重新设置着色器
	if (!d->screenInfoPrepared)
	{
		GMDx11EffectVariableBank& bank = getVarBank();
		const GMWindowStates& windowStates = getContext()->getWindow()->getWindowStates();
		ID3DX11EffectVariable* screenInfo = d->effect->GetVariableByName(GM_VariablesDesc.ScreenInfoAttributes.ScreenInfo.toStdString().c_str());
		CHECK_VAR(screenInfo);

		ID3DX11EffectScalarVariable* screenWidth = bank.ScreenWidth();
		GM_DX_TRY(screenWidth, screenWidth->SetInt(windowStates.renderRect.width));

		ID3DX11EffectScalarVariable* screenHeight = bank.ScreenHeight();
		GM_DX_TRY(screenHeight, screenHeight->SetInt(windowStates.renderRect.height));

		ID3DX11EffectScalarVariable* multisampling = bank.ScreenMultiSampling();
		GM_DX_TRY(multisampling, multisampling->SetBool(windowStates.sampleCount > 1));
		d->screenInfoPrepared = true;
	}
}

void GMDx11Technique::prepareTextures(GMModel* model)
{
	D(d);
	GM_FOREACH_ENUM_CLASS(type, GMTextureType::Ambient, GMTextureType::EndOfCommonTexture)
	{
		GMTextureSampler& sampler = model->getShader().getTextureList().getTextureSampler(type);
		// 写入纹理属性，如是否绘制，偏移等
		GMTextureAsset texture = getTexture(sampler);
		applyTextureAttribute(model, texture, type);

		if (!texture.isEmpty() && !d->engine->isNeedDiscardTexture(model, type))
		{
			// 激活动画序列
			texture.getTexture()->bindSampler(&sampler);
			texture.getTexture()->useTexture((GMint32)type);
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
				GMTextureAsset whiteTexture = getWhiteTexture();
				applyTextureAttribute(model, whiteTexture, type);
				whiteTexture.getTexture()->bindSampler(nullptr);
				whiteTexture.getTexture()->useTexture((GMint32)type);
			}
		}
	}

	const GMDx11CubeMapState& cubeMapState = getEngine()->getCubeMapState();
	if (cubeMapState.hasCubeMap)
	{
		GM_ASSERT(cubeMapState.model && cubeMapState.cubeMapTechnique);
		cubeMapState.cubeMapTechnique->prepareTextures(cubeMapState.model);
	}
}

void GMDx11Technique::applyTextureAttribute(GMModel* model, GMTextureAsset texture, GMTextureType type)
{
	D(d);
	
	GMDx11EffectVariableBank& effectBank = getVarBank();
	ID3DX11EffectVariable* textureVariable = nullptr;

	switch (type)
	{
	case GMTextureType::Ambient:
		textureVariable = effectBank.AmbientTexture();
		break;
	case GMTextureType::Diffuse:
		textureVariable = effectBank.DiffuseTexture();
		break;
	case GMTextureType::Specular:
		textureVariable = effectBank.SpecularTexture();
		break;
	case GMTextureType::NormalMap:
		textureVariable = effectBank.NormalMapTexture();
		break;
	case GMTextureType::Lightmap:
		textureVariable = effectBank.LightMapTexture();
		break;
	case GMTextureType::CubeMap:
		textureVariable = effectBank.CubeMapTexture();
		break;
	case GMTextureType::Albedo:
		textureVariable = effectBank.AlbedoTexture();
		break;
	case GMTextureType::MetallicRoughnessAO:
		textureVariable = effectBank.MetallicRoughnessAOTexture();
		break;
	default:
		GM_ASSERT(false);
		return;
	}

	const GMTextureAttributeBank* bank = nullptr;
	{
		auto iter = d->textureVariables.find(textureVariable);
		if (iter != d->textureVariables.end())
		{
			bank = &iter->second;
		}
		else
		{
			GMTextureAttributeBank newBank;
			newBank.enabled = textureVariable->GetMemberByName(GM_VariablesDesc.TextureAttributes.Enabled.toStdString().c_str())->AsScalar();
			newBank.offsetX = textureVariable->GetMemberByName(GM_VariablesDesc.TextureAttributes.OffsetX.toStdString().c_str())->AsScalar();
			newBank.offsetY = textureVariable->GetMemberByName(GM_VariablesDesc.TextureAttributes.OffsetY.toStdString().c_str())->AsScalar();
			newBank.scaleX = textureVariable->GetMemberByName(GM_VariablesDesc.TextureAttributes.ScaleX.toStdString().c_str())->AsScalar();
			newBank.scaleY = textureVariable->GetMemberByName(GM_VariablesDesc.TextureAttributes.ScaleY.toStdString().c_str())->AsScalar();
			d->textureVariables[textureVariable] = newBank;
			bank = &newBank;
		}
	}

	if (!texture.isEmpty())
	{
		GM_DX_TRY(bank->enabled, bank->enabled->SetBool(TRUE));
		GM_DX_TRY(bank->offsetX, bank->offsetX->SetFloat(0.f));
		GM_DX_TRY(bank->offsetY, bank->offsetY->SetFloat(0.f));
		GM_DX_TRY(bank->scaleX, bank->scaleX->SetFloat(1.f));
		GM_DX_TRY(bank->scaleY, bank->scaleY->SetFloat(1.f));

		auto applyCallback = [&](GMS_TextureTransformType type, Pair<GMfloat, GMfloat>&& args) {
			if (type == GMS_TextureTransformType::Scale)
			{
				GM_DX_TRY(bank->scaleX, bank->scaleX->SetFloat(args.first));
				GM_DX_TRY(bank->scaleY, bank->scaleY->SetFloat(args.second));
			}
			else if (type == GMS_TextureTransformType::Scroll)
			{
				GM_DX_TRY(bank->offsetX, bank->offsetX->SetFloat(args.first));
				GM_DX_TRY(bank->offsetY, bank->offsetY->SetFloat(args.second));
			}
			else
			{
				GM_ASSERT(false);
			}
		};

		model->getShader().getTextureList().getTextureSampler(type).applyTexMode(GM.getRunningStates().elapsedTime, applyCallback);
	}
	else
	{
		// 将这个Texture的Enabled设置为false
		GM_DX_TRY(bank->enabled, bank->enabled->SetBool(FALSE));
	}
}

void GMDx11Technique::prepareShadow(bool isDrawingShadow)
{
	D(d);
	const GMShadowSourceDesc& shadowSourceDesc = getEngine()->getShadowSourceDesc();
	GMFloat4 viewPosition;
	shadowSourceDesc.position.loadFloat4(viewPosition);

	// 设置变量
	GMDx11EffectVariableBank& bank = getVarBank();
	ID3DX11EffectVariable* shadowInfo = bank.ShadowInfo();
	ID3DX11EffectVectorVariable* position = bank.ShadowPosition();
	ID3DX11EffectScalarVariable* shadowMapWidth = bank.ShadowMapWidth();
	ID3DX11EffectScalarVariable* shadowMapHeight = bank.ShadowMapHeight();
	ID3DX11EffectScalarVariable* biasMin = bank.ShadowBiasMin();
	ID3DX11EffectScalarVariable* biasMax = bank.ShadowBiasMax();

	if (!isDrawingShadow) // 只有非绘制阴影时，才需要阴影贴图相关信息
	{
		if (d->lastShadowVersion != shadowSourceDesc.version)
		{
			const GMWindowStates& windowStates = d->context->getWindow()->getWindowStates();

			GM_DX_HR(position->SetFloatVector(ValuePointer(viewPosition)));

			GM_DX_HR(biasMin->SetFloat(shadowSourceDesc.biasMin));
			GM_DX_HR(biasMax->SetFloat(shadowSourceDesc.biasMax));

			GMDx11ShadowFramebuffers* shadowFramebuffers = gm_cast<GMDx11ShadowFramebuffers*>(getEngine()->getShadowMapFramebuffers());
			GM_DX_HR(shadowMapWidth->SetInt(shadowFramebuffers->getShadowMapWidth()));
			GM_DX_HR(shadowMapHeight->SetInt(shadowFramebuffers->getShadowMapHeight()));

			ID3DX11EffectShaderResourceVariable* shadowMap = windowStates.sampleCount > 1 ? bank.ShadowMapMSAA() : bank.ShadowMap();
			GM_DX_HR(shadowMap->SetResource(shadowFramebuffers->getShadowMapShaderResourceView()));

			d->lastShadowVersion = shadowSourceDesc.version;
		}
	}

	ID3DX11EffectScalarVariable* currentCascadeLevel = bank.CurrentCascadeLevel();
	ID3DX11EffectScalarVariable* viewCascade = bank.ViewCascade();

	// 是否显示CSM范围
	GMRenderConfig config = GM.getConfigs().getConfig(gm::GMConfigs::Render).asRenderConfig();
	bool vc = config.get(GMRenderConfigs::ViewCascade_Bool).toBool();
	GM_DX_HR(viewCascade->SetBool(vc ? TRUE : FALSE));

	// 设置当前的Cascade层级和Shadow Map矩阵
	ICSMFramebuffers* csm = getEngine()->getCSMFramebuffers();
	GMCascadeLevel currentLevel = csm->currentLevel();
	GM_DX_HR(currentCascadeLevel->SetInt(currentLevel));
}

void GMDx11Technique::prepareBuffer(GMModel* model)
{
	D(d);
	GMuint32 stride = sizeof(GMVertex);
	GMuint32 offset = 0;
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

void GMDx11Technique::prepareLights()
{
	D(d);
	getEngine()->activateLights(this);
}

void GMDx11Technique::prepareRasterizer(GMModel* model)
{
	D(d);
	const GMWindowStates& windowStates = d->context->getWindow()->getWindowStates();
	bool multisampleEnable = windowStates.sampleCount > 1;
	if (!d->rasterizer)
	{
		d->rasterizer = d->effect->GetVariableByName(GM_VariablesDesc.RasterizerState.toStdString().c_str())->AsRasterizer();
	}

	if (!d->rasterizerStates)
		d->rasterizerStates = gm_makeOwnedPtr<GMDx11RasterizerStates>(getContext());

	GM_ASSERT(d->rasterizer);
	GM_DX_HR(d->rasterizer->SetRasterizerState(
		0, 
		d->rasterizerStates->getRasterStates(model)
	));
}

void GMDx11Technique::prepareMaterials(GMModel* model)
{
	D(d);
	GMDx11EffectVariableBank& bank = getVarBank();
	const GMShader& shader = model->getShader();
	const GMMaterial& material = shader.getMaterial();
	GM_DX_TRY(bank.Ka(), bank.Ka()->SetFloatVector(ValuePointer(material.getAmbient())));
	GM_DX_TRY(bank.Kd(), bank.Kd()->SetFloatVector(ValuePointer(material.getDiffuse())));
	GM_DX_TRY(bank.Ks(), bank.Ks()->SetFloatVector(ValuePointer(material.getSpecular())));
	GM_DX_TRY(bank.Shininess(), bank.Shininess()->SetFloat(material.getShininess()));
	GM_DX_TRY(bank.Refreactivity(), bank.Refreactivity()->SetFloat(material.getRefractivity()));
	GM_DX_TRY(bank.F0(), bank.F0()->SetFloatVector(ValuePointer(material.getF0())));

	IShaderProgram* shaderProgram = getEngine()->getShaderProgram();
	GMIlluminationModel illuminationModel = shader.getIlluminationModel();
	shaderProgram->setInt(VI(IlluminationModel), (GMint32)illuminationModel);
}

void GMDx11Technique::prepareBlend(GMModel* model)
{
	D(d);
	if (!d->blend)
	{
		d->blend = d->effect->GetVariableByName(GM_VariablesDesc.BlendState.toStdString().c_str())->AsBlend();
	}
	GM_ASSERT(d->blend);

	const GMGlobalBlendStateDesc& globalBlendState = getEngine()->getGlobalBlendState();
	if (!d->blendStates)
		d->blendStates = gm_makeOwnedPtr<GMDx11BlendStates>(getContext());

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

void GMDx11Technique::prepareDepthStencil(GMModel* model)
{
	D(d);
	if (!d->depthStencil)
	{
		d->depthStencil = d->effect->GetVariableByName(GM_VariablesDesc.DepthStencilState.toStdString().c_str())->AsDepthStencil();
	}
	GM_ASSERT(d->depthStencil);

	if (!d->depthStencilStates)
		d->depthStencilStates = gm_makeOwnedPtr<GMDx11DepthStencilStates>(getContext());
	
	// 指定需要深度测试，或者没有混合的情况下，开启深度测试
	bool depthEnabled = !model->getShader().getNoDepthTest();
	GM_DX_HR(d->depthStencil->SetDepthStencilState(
		0,
		d->depthStencilStates->getDepthStencilState(
			depthEnabled,
			getEngine()->getStencilOptions()
		)
	));
}

void GMDx11Technique::prepareDebug(GMModel* model)
{
	D(d);
	GMint32 mode = d->debugConfig.get(gm::GMDebugConfigs::DrawPolygonNormalMode).toInt();
	
	IShaderProgram* shaderProgram = getEngine()->getShaderProgram();
	shaderProgram->setInt(VI(Debug.Normal), mode);
}

GMTextureAsset GMDx11Technique::getTexture(GMTextureSampler& sampler)
{
	D(d);
	if (sampler.getFrameCount() == 0)
		return GMAsset::invalidAsset();

	if (sampler.getFrameCount() == 1)
		return sampler.getFrameByIndex(0);

	// 如果frameCount > 1，说明是个动画，要根据Shader的间隔来选择合适的帧
	// TODO
	GMint32 elapsed = GM.getRunningStates().elapsedTime * 1000;

	return sampler.getFrameByIndex((elapsed / sampler.getAnimationMs()) % sampler.getFrameCount());
}

void GMDx11Technique::setGamma(IShaderProgram* shaderProgram)
{
	D(d);
	bool needGammaCorrection = getEngine()->needGammaCorrection();
	shaderProgram->setBool(VI(GammaCorrection.GammaCorrection), needGammaCorrection);
	GMfloat gamma = getEngine()->getGammaValue();
	if (gamma != d->gamma)
	{
		shaderProgram->setFloat(VI(GammaCorrection.GammaValue), gamma);
		shaderProgram->setFloat(VI(GammaCorrection.GammaInvValue), 1.f / gamma);
		d->gamma = gamma;
	}
}

void GMDx11Technique::draw(GMModel* model)
{
	prepareScreenInfo();
	prepareBuffer(model);
	prepareLights();
	prepareMaterials(model);
	prepareRasterizer(model);
	prepareBlend(model);
	prepareDepthStencil(model);
	prepareTextures(model);
	prepareDebug(model);
	passAllAndDraw(model);
}

void GMDx11Technique::passAllAndDraw(GMModel* model)
{
	D(d);
	D3DX11_TECHNIQUE_DESC techDesc;
	GM_DX_HR(getTechnique()->GetDesc(&techDesc));

	for (GMuint32 p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = getTechnique()->GetPassByIndex(p);
		pass->Apply(0, d->deviceContext);
		if (model->getDrawMode() == GMModelDrawMode::Vertex)
			d->deviceContext->Draw(gm_sizet_to<UINT>(model->getVerticesCount()), 0);
		else
			d->deviceContext->DrawIndexed(gm_sizet_to<UINT>(model->getVerticesCount()), 0, 0);
	}
}

ID3DX11EffectTechnique* GMDx11Technique::getTechnique()
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

void GMDx11Technique_2D::prepareTextures(GMModel* model)
{
	D(d);
	GM_FOREACH_ENUM_CLASS(type, GMTextureType::Ambient, GMTextureType::EndOfCommonTexture)
	{
		GMTextureSampler& sampler = model->getShader().getTextureList().getTextureSampler(type);
		// 写入纹理属性，如是否绘制，偏移等
		GMTextureAsset texture = getTexture(sampler);
		applyTextureAttribute(model, texture, type);
		if (!texture.isEmpty())
		{
			texture.getTexture()->bindSampler(&model->getShader().getTextureList().getTextureSampler(type));
			texture.getTexture()->useTexture((GMint32)type);
		}
	}
}

void GMDx11Technique_CubeMap::prepareTextures(GMModel* model)
{
	GMTextureSampler& sampler = model->getShader().getTextureList().getTextureSampler(GMTextureType::CubeMap);
	// 写入纹理属性，如是否绘制，偏移等
	GMTextureAsset texture = getTexture(sampler);
	applyTextureAttribute(model, texture, GMTextureType::CubeMap);
	if (!texture.isEmpty())
	{
		texture.getTexture()->bindSampler(&model->getShader().getTextureList().getTextureSampler(GMTextureType::CubeMap));
		texture.getTexture()->useTexture((GMint32)GMTextureType::CubeMap);
		GMDx11CubeMapState& cubeMapState = getEngine()->getCubeMapState();
		if (cubeMapState.model != model)
		{
			cubeMapState.hasCubeMap = true;
			cubeMapState.cubeMapTechnique = this;
			cubeMapState.model = model;
		}
	}
}

GMDx11Technique_Filter::GMDx11Technique_Filter(const IRenderContext* context)
	: GMDx11Technique(context)
{
	setHDR(getEngine()->getShaderProgram());
}

void GMDx11Technique_Filter::draw(GMModel* model)
{
	D(d);
	prepareScreenInfo();
	prepareBuffer(model);
	prepareRasterizer(model);
	prepareBlend(model);
	prepareDepthStencil(model);
	passAllAndDraw(model);
}

void GMDx11Technique_Filter::passAllAndDraw(GMModel* model)
{
	D_BASE(d, Base);
	const GMWindowStates& windowStates = d->context->getWindow()->getWindowStates();
	D3DX11_TECHNIQUE_DESC techDesc;
	GM_DX_HR(getTechnique()->GetDesc(&techDesc));

	for (GMuint32 p = 0; p < techDesc.Passes; ++p)
	{
		GMTextureAsset filterTextureAsset = model->getShader().getTextureList().getTextureSampler(GMTextureType::Ambient).getFrameByIndex(0);
		GM_ASSERT(!filterTextureAsset.isEmpty());
		GMDx11Texture* filterTexture = filterTextureAsset.get<GMDx11Texture*>();
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

		if (model->getDrawMode() == GMModelDrawMode::Vertex)
			d->deviceContext->Draw(gm_sizet_to<UINT>(model->getVerticesCount()), 0);
		else
			d->deviceContext->DrawIndexed(gm_sizet_to<UINT>(model->getVerticesCount()), 0, 0);
	}
}

void GMDx11Technique_Filter::beginModel(GMModel* model, const GMGameObject* parent)
{
	D(d);
	D_BASE(db, Base);
	GMDx11Technique::beginModel(model, parent);

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
	bool b = shaderProgram->setInterfaceInstance(GM_VariablesDesc.FilterAttributes.Filter.toStdString().c_str(), GM_VariablesDesc.FilterAttributes.Types[filterMode].toStdString().c_str(), GMShaderType::Effect);
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
			shaderProgram->setBool(VI_B(HDR.HDR), false);
		}
	}
}

void GMDx11Technique_Filter::setHDR(IShaderProgram* shaderProgram)
{
	D(d);
	D_BASE(db, Base);
	shaderProgram->setBool(VI_B(HDR.HDR), true);
	if (d->state.toneMapping == GMToneMapping::Reinhard)
	{
		bool b = shaderProgram->setInterfaceInstance(GM_VariablesDesc.HDR.ToneMapping.toStdString().c_str(), "ReinhardToneMapping", GMShaderType::Effect);
		GM_ASSERT(b);
	}
	else
	{
		GM_ASSERT(false);
		gm_warning(gm_dbg_wrap("Invalid tonemapping."));
	}
}

void GMDx11Technique_Deferred_3D::passAllAndDraw(GMModel* model)
{
	D(d);
	D3DX11_TECHNIQUE_DESC techDesc;
	GM_DX_HR(getTechnique()->GetDesc(&techDesc));

	IGBuffer* gbuffer = getEngine()->getGBuffer();
	IFramebuffers* framebuffers = gbuffer->getGeometryFramebuffers();
	for (GMuint32 p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = getTechnique()->GetPassByIndex(p);
		pass->Apply(0, d->deviceContext);

		GM_ASSERT(framebuffers);
		framebuffers->bind();
		if (model->getDrawMode() == GMModelDrawMode::Vertex)
			d->deviceContext->Draw(gm_sizet_to<UINT>(model->getVerticesCount()), 0);
		else
			d->deviceContext->DrawIndexed(gm_sizet_to<UINT>(model->getVerticesCount()), 0, 0);
		framebuffers->unbind();
	}
}

void GMDx11Technique_Deferred_3D_LightPass::passAllAndDraw(GMModel* model)
{
	D(d);
	D3DX11_TECHNIQUE_DESC techDesc;
	GM_DX_HR(getTechnique()->GetDesc(&techDesc));
	for (GMuint32 p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = getTechnique()->GetPassByIndex(p);
		pass->Apply(0, d->deviceContext);
		if (model->getDrawMode() == GMModelDrawMode::Vertex)
			d->deviceContext->Draw(gm_sizet_to<UINT>(model->getVerticesCount()), 0);
		else
			d->deviceContext->DrawIndexed(gm_sizet_to<UINT>(model->getVerticesCount()), 0, 0);
	}
}

void GMDx11Technique_Deferred_3D_LightPass::prepareTextures(GMModel* model)
{
	D(d);
	GMDx11GBuffer* gbuffer = gm_cast<GMDx11GBuffer*>(getEngine()->getGBuffer());
	gbuffer->useGeometryTextures(d->effect);

	const GMDx11CubeMapState& cubeMapState = getEngine()->getCubeMapState();
	if (cubeMapState.hasCubeMap)
	{
		GM_ASSERT(cubeMapState.model && cubeMapState.cubeMapTechnique);
		cubeMapState.cubeMapTechnique->prepareTextures(cubeMapState.model);
	}
}

void GMDx11Technique_3D_Shadow::beginModel(GMModel* model, const GMGameObject* parent)
{
	D_BASE(d, GMDx11Technique);
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

	// Technique决定自己的顶点Layout
	d->deviceContext->IASetInputLayout(d->inputLayout);
	d->deviceContext->IASetPrimitiveTopology(getMode(model->getPrimitiveTopologyMode()));

	if (parent)
	{
		shaderProgram->setMatrix4(VI(ModelMatrix), parent->getTransform());
		shaderProgram->setMatrix4(VI(InverseTransposeModelMatrix), InverseTranspose(parent->getTransform()));
	}
	else
	{
		shaderProgram->setMatrix4(VI(ModelMatrix), Identity<GMMat4>());
		shaderProgram->setMatrix4(VI(InverseTransposeModelMatrix), Identity<GMMat4>());
	}

	prepareShadow(true);
}

void GMDx11Technique_3D_Shadow::draw(GMModel* model)
{
	prepareBuffer(model);
	prepareRasterizer(model);
	prepareBlend(model);
	prepareDepthStencil(model);
	passAllAndDraw(model);
}

ID3DX11EffectTechnique* GMDx11Technique_Custom::getTechnique()
{
	GMComPtr<ID3DX11Effect> effect;
	IShaderProgram* shaderProgram = getEngine()->getShaderProgram();
	shaderProgram->getInterface(GameMachineInterfaceID::D3D11Effect, (void**)&effect);
	GM_ASSERT(effect);
	return effect->GetTechniqueByName(getTechniqueName());
}

const char* GMDx11Technique_Custom::getTechniqueName()
{
	GM_ASSERT(getCurrentModel());
	return getTechniqueNameByTechniqueId(getCurrentModel()->getTechniqueId()).c_str();
}