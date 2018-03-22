#include "stdafx.h"
#include "gmdx11renderers.h"
#include "gmdata/gmmodel.h"
#include <gmgameobject.h>
#include <gmdx11helper.h>
#include "gmdx11modelpainter.h"

#define GMSHADER_LAYOUT_NAME_POSITION "POSITION"
#define GMSHADER_LAYOUT_NAME_NORMAL "NORMAL"
#define GMSHADER_LAYOUT_NAME_TEXCOORD "TEXCOORD"
#define GMSHADER_LAYOUT_NAME_TANGENT "TEXCOORD"
#define GMSHADER_LAYOUT_NAME_BITANGENT "TEXCOORD"
#define GMSHADER_LAYOUT_NAME_LIGHTMAP "TEXCOORD"
#define FLOAT_OFFSET(i) (sizeof(gm::GMfloat) * i)

namespace
{
	D3D11_INPUT_ELEMENT_DESC GMSHADER_ElementDescriptions[] =
	{
		{ GMSHADER_LAYOUT_NAME_POSITION, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, FLOAT_OFFSET(0), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 3

		{ GMSHADER_LAYOUT_NAME_NORMAL, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, FLOAT_OFFSET(3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 3

		{ GMSHADER_LAYOUT_NAME_TEXCOORD, 0, DXGI_FORMAT_R32G32_FLOAT, 0, FLOAT_OFFSET(6), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 2
	};

	D3D_PRIMITIVE_TOPOLOGY getMode(GMMesh* obj)
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

	D3D11_RASTERIZER_DESC getRasterizerDesc(
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

	struct GMDx11RasterStates : public GMSingleton<GMDx11RasterStates>
	{
		enum
		{
			Size_Cull = 2,
			Size_FrontFace = 2,
		};

		//TODO 先不考虑FillMode
	public:
		GMDx11RasterStates::GMDx11RasterStates()
		{
			engine = gm_static_cast<GMDx11GraphicEngine*>(GM.getGraphicEngine());
		}

		~GMDx11RasterStates()
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
}

GMDx11Renderer::GMDx11Renderer()
{
	D(d);
	IShaderProgram* shaderProgram = getEngine()->getShaderProgram();
	shaderProgram->useProgram();
	if (!d->effect)
	{
		shaderProgram->getInterface(GameMachineInterfaceID::D3D11Effect, (void**)&d->effect);
		GM_ASSERT(d->effect);
	}
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
	
	const GMShaderVariablesDesc& desc = shaderProgram->getDesc();
	shaderProgram->setMatrix4(desc.ModelMatrix, Transpose(parent->getTransform()));
	shaderProgram->setMatrix4(desc.ViewMatrix, Transpose(GM.getCamera().getFrustum().getViewMatrix()));
	shaderProgram->setMatrix4(desc.ProjectionMatrix, Transpose(GM.getCamera().getFrustum().getProjectionMatrix()));
}

void GMDx11Renderer::endModel()
{
}

void GMDx11Renderer::drawTextures()
{
	D(d);
	GM_ASSERT(d->shader);
	GM_FOREACH_ENUM_CLASS(type, GMTextureType::AMBIENT, GMTextureType::END)
	{
		GMint count = GMMaxTextureCount(type);
		for (GMint i = 0; i < count; i++)
		{
			GMTextureFrames& textures = d->shader->getTexture().getTextureFrames(type, i);

			// 获取序列中的这一帧
			ITexture* texture = getTexture(textures);
			if (texture)
			{
				// 激活动画序列
				texture->drawTexture(&textures);
			}
		}
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

	GMDx11RasterStates& rasterStates = GMDx11RasterStates::instance();
	GM_ASSERT(d->rasterizer);
	GM_DX_HR(d->rasterizer->SetRasterizerState(
		0, 
		rasterStates.getRasterStates(d->shader->getFrontFace(), d->shader->getCull())
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
	prepareRasterizer(component);
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