#include "stdafx.h"
#include "gmdx11renderers.h"
#include "gmdata/gmmodel.h"
#include <gmgameobject.h>
#include <gmdx11helper.h>
#include "gmdx11modelpainter.h"
#include "gmdx11texture.h"

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
	constexpr GMint GMDX11_MAX_LIGHT_COUNT = 10; //灯光最大数量

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

	inline D3D_PRIMITIVE_TOPOLOGY getMode(GMMesh* obj)
	{
		switch (obj->getArrangementMode())
		{
		case GMArrangementMode::Triangle_Fan:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case GMArrangementMode::Triangle_Strip:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		case GMArrangementMode::Triangles:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		case GMArrangementMode::Lines:
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
			engine = gm_static_cast<GMDx11GraphicEngine*>(GM.getGraphicEngine());
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
			engine = gm_static_cast<GMDx11GraphicEngine*>(GM.getGraphicEngine());
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
			engine = gm_static_cast<GMDx11GraphicEngine*>(GM.getGraphicEngine());
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

GMDx11Renderer::GMDx11Renderer()
{
	D(d);
	IShaderProgram* shaderProgram = getEngine()->getShaderProgram();
	shaderProgram->useProgram();
	GM_ASSERT(!d->effect);
	shaderProgram->getInterface(GameMachineInterfaceID::D3D11Effect, (void**)&d->effect);
	GM_ASSERT(d->effect);
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
	ID3D11DeviceContext* context = getEngine()->getDeviceContext();
	context->IASetInputLayout(d->inputLayout);
	context->IASetPrimitiveTopology(getMode(model->getMesh()));
	
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
	shaderProgram->setMatrix4(desc->ViewMatrix, GM.getCamera().getFrustum().getViewMatrix());
	shaderProgram->setMatrix4(desc->ProjectionMatrix, GM.getCamera().getFrustum().getProjectionMatrix());
}

void GMDx11Renderer::endModel()
{
}

void GMDx11Renderer::prepareTextures()
{
	D(d);
	GM_ASSERT(d->shader);
	GM_FOREACH_ENUM_CLASS(type, GMTextureType::AMBIENT, GMTextureType::END)
	{
		GMint count = GMMaxTextureCount(type);
		for (GMint i = 0; i < count; i++)
		{
			GMTextureFrames& textures = d->shader->getTexture().getTextureFrames(type, i);
			// 写入纹理属性，如是否绘制，偏移等
			applyTextureAttribute(getTexture(textures), type, i);
		}
	}
}

void GMDx11Renderer::drawTextures()
{
	D(d);
	GM_ASSERT(d->shader);
	GMint registerId = 0;
	GM_FOREACH_ENUM_CLASS(type, GMTextureType::AMBIENT, GMTextureType::END)
	{
		GMint count = GMMaxTextureCount(type);
		for (GMint i = 0; i < count; ++i, ++registerId)
		{
			GMTextureFrames& textures = d->shader->getTexture().getTextureFrames(type, i);

			// 获取序列中的这一帧
			ITexture* texture = getTexture(textures);
			if (texture)
			{
				// 激活动画序列
				texture->drawTexture(&textures, registerId);
			}
		}
	}
}

void GMDx11Renderer::applyTextureAttribute(ITexture* texture, GMTextureType type, GMint index)
{
	D(d);
	const GMShaderVariablesDesc* desc = getVariablesDesc();
	ID3DX11EffectVariable* texAttrs = nullptr;
	const char* textureName = nullptr;
	if (type == GMTextureType::AMBIENT)
	{
		textureName = desc->AmbientTextureName;
	}
	else if (type == GMTextureType::DIFFUSE)
	{
		textureName = desc->DiffuseTextureName;
	}
	else if (type == GMTextureType::NORMALMAP)
	{
		textureName = desc->NormalMapTextureName;
	}
	else if (type == GMTextureType::LIGHTMAP)
	{
		textureName = desc->LightMapTextureName;
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
				GM_DX_HR(offsetX->SetFloat(args.first));
				GM_DX_HR(offsetY->SetFloat(args.second));
			}
			else if (type == GMS_TextureModType::SCROLL)
			{
				GM_DX_HR(scaleX->SetFloat(args.first));
				GM_DX_HR(scaleY->SetFloat(args.second));
			}
			else
			{
				GM_ASSERT(false);
			}
		};

		d->shader->getTexture().getTextureFrames(type, index).applyTexMode(GM.getGameTimeSeconds(), applyCallback);
	}
	else
	{
		// 将这个Texture的Enabled设置为false
		ID3DX11EffectScalarVariable* enabled = texAttrs->GetMemberByName(desc->TextureAttributes.Enabled)->AsScalar();
		CHECK_VAR(enabled);
		enabled->SetBool(FALSE);
	}
}

void GMDx11Renderer::prepareBuffer(IQueriable* painter)
{
	GMuint stride = sizeof(GMDx11VertexData);
	GMuint offset = 0;
	GMComPtr<ID3D11Buffer> buffer;
	painter->getInterface(GameMachineInterfaceID::D3D11Buffer, (void**)&buffer);
	GM_ASSERT(buffer);
	getEngine()->getDeviceContext()->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
}

void GMDx11Renderer::prepareLights()
{
	D(d);
	if (getEngine()->needActivateLight())
	{
		const GMShaderVariablesDesc& svd = getEngine()->getShaderProgram()->getDesc();
		const Vector<GMLight>& lights = getEngine()->getLights();
		GMuint indices[(GMuint)GMLightType::COUNT] = { 0 };
		const GMShaderVariablesLightDesc* lightAttrs[] = { &svd.AmbientLight, &svd.SpecularLight };
		for (auto& light : lights)
		{
			const GMShaderVariablesLightDesc* lightAttr = lightAttrs[(GMuint)light.getType()];
			ID3DX11EffectVariable* lightAttributeVar = d->effect->GetVariableByName(lightAttr->Name)->GetElement(indices[(GMuint)light.getType()]++);
			GM_ASSERT(lightAttributeVar->IsValid());
			ID3DX11EffectVectorVariable* position = lightAttributeVar->GetMemberByName(svd.LightAttributes.Position)->AsVector();
			GM_ASSERT(position->IsValid());
			ID3DX11EffectVectorVariable* color = lightAttributeVar->GetMemberByName(svd.LightAttributes.Color)->AsVector();
			GM_ASSERT(color->IsValid());
			GM_DX_HR(position->SetFloatVector(light.getLightPosition()));
			GM_DX_HR(color->SetFloatVector(light.getLightColor()));
		}

		GM_FOREACH_ENUM_CLASS(type, GMLightType::AMBIENT, GMLightType::COUNT)
		{
			ID3DX11EffectScalarVariable* lightCount = d->effect->GetVariableByName(lightAttrs[(GMuint)type]->Count)->AsScalar();
			GM_ASSERT(lightCount->IsValid());
			GM_ASSERT(indices[(GMuint)type] < GMDX11_MAX_LIGHT_COUNT);
			GM_DX_HR(lightCount->SetInt(indices[(GMuint)type]));
		}
	}
}

void GMDx11Renderer::prepareRasterizer(GMComponent* component)
{
	D(d);
	ID3D11DeviceContext* context = getEngine()->getDeviceContext();
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
		rasterStates.getRasterStates(d->shader->getFrontFace(), d->shader->getCull())
	));
}

void GMDx11Renderer::prepareMaterials(GMComponent* component)
{
	D(d);
	const GMShaderVariablesDesc& svd = getEngine()->getShaderProgram()->getDesc();
	ID3D11DeviceContext* context = getEngine()->getDeviceContext();
	const GMMaterial& material = component->getShader().getMaterial();
	ID3DX11EffectVariable* materialVar = d->effect->GetVariableByName(svd.MaterialName);
	GM_ASSERT(materialVar->IsValid());
	GM_DX_HR(materialVar->GetMemberByName(svd.MaterialAttributes.Ka)->AsVector()->SetFloatVector(ValuePointer(material.ka)));
	GM_DX_HR(materialVar->GetMemberByName(svd.MaterialAttributes.Kd)->AsVector()->SetFloatVector(ValuePointer(material.kd)));
	GM_DX_HR(materialVar->GetMemberByName(svd.MaterialAttributes.Ks)->AsVector()->SetFloatVector(ValuePointer(material.ks)));
	GM_DX_HR(materialVar->GetMemberByName(svd.MaterialAttributes.Shininess)->AsScalar()->SetFloat(material.shininess));
	GM_DX_HR(materialVar->GetMemberByName(svd.MaterialAttributes.Refreactivity)->AsScalar()->SetFloat(material.refractivity));
}

void GMDx11Renderer::prepareBlend(GMComponent* component)
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
		if (d->shader->getBlend())
		{
			GM_DX_HR(d->blend->SetBlendState(
				0,
				blendStates.getBlendState(true, d->shader->getBlendFactorSource(), d->shader->getBlendFactorDest())
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
		if (d->shader->getBlend())
		{
			GM_DX_HR(d->blend->SetBlendState(
				0,
				blendStates.getBlendState(true, d->shader->getBlendFactorSource(), d->shader->getBlendFactorDest())
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

void GMDx11Renderer::prepareDepthStencil(GMComponent* component)
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
			!d->shader->getNoDepthTest(),
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

void GMDx11Renderer::draw(IQueriable* painter, GMComponent* component, GMMesh* mesh)
{
	D(d);
	d->shader = &component->getShader();
	prepareBuffer(painter);
	prepareLights();
	prepareMaterials(component);
	prepareRasterizer(component);
	prepareBlend(component);
	prepareDepthStencil(component);
	passAllAndDraw(component);
}

void GMDx11Renderer::passAllAndDraw(GMComponent* component)
{
	D(d);
	D3DX11_TECHNIQUE_DESC techDesc;
	GM_DX_HR(getTechnique()->GetDesc(&techDesc));

	GMuint primitiveCount = component->getPrimitiveCount();
	GMuint* offsetPtr = component->getOffsetPtr();
	GMuint* vertexCountPtr = component->getPrimitiveVerticesCountPtr();
	for (GMuint p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = getTechnique()->GetPassByIndex(p);
		prepareTextures();
		pass->Apply(0, getEngine()->getDeviceContext());
		drawTextures();
		for (GMuint i = 0; i < primitiveCount; ++i)
		{
			getEngine()->getDeviceContext()->Draw(vertexCountPtr[i], offsetPtr[i]);
		}
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