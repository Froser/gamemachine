#include "stdafx.h"
#include "gmdx11renderers.h"
#include "gmdata/gmmodel.h"
#include <gmgameobject.h>
#include "shader_constants.h"

namespace
{
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
		void applyRasterStates(GMS_Cull cullMode, GMS_FrontFace frontFace)
		{
			if (!states[(GMuint)cullMode][(GMuint)frontFace])
			{
				D3D11_RASTERIZER_DESC desc = getDefaultDesc();
				desc.CullMode = cullMode == GMS_Cull::CULL ? D3D11_CULL_BACK : D3D11_CULL_NONE;
				desc.FrontCounterClockwise = frontFace == GMS_FrontFace::CLOCKWISE ? FALSE : TRUE;
				createRasterizerState(desc, &states[(GMuint)cullMode][(GMuint)frontFace]);
			}

			GM_ASSERT(states[(GMuint)cullMode][(GMuint)frontFace]);
			engine->getDeviceContext()->RSSetState(states[(GMuint)cullMode][(GMuint)frontFace]);
		}

	private:
		D3D11_RASTERIZER_DESC getDefaultDesc()
		{
			D3D11_RASTERIZER_DESC desc;
			desc.FillMode = D3D11_FILL_SOLID;
			desc.CullMode = D3D11_CULL_BACK;
			desc.FrontCounterClockwise = FALSE;
			desc.DepthBias = 0;
			desc.SlopeScaledDepthBias = 0.0f;
			desc.DepthBiasClamp = 0.0f;
			desc.DepthClipEnable = TRUE;
			desc.ScissorEnable = FALSE;
			desc.MultisampleEnable = GM.getGameMachineRunningStates().sampleCount > 1;
			desc.AntialiasedLineEnable = desc.MultisampleEnable;
			return desc;
		}

		bool createRasterizerState(const D3D11_RASTERIZER_DESC& desc, ID3D11RasterizerState** out)
		{
			HRESULT hr = engine->getDevice()->CreateRasterizerState(&desc, out);
			GM_COM_CHECK_RETURN(hr, false);
			return SUCCEEDED(hr);
		}

	private:
		GMDx11GraphicEngine* engine = nullptr;
		ID3D11RasterizerState* states[Size_Cull][Size_FrontFace] = { 0 };
	};
}

GMDx11Renderer::GMDx11Renderer()
{
	D(d);
	// 定义顶点布局
	GM_ASSERT(!d->inputLayout);
	HRESULT hr;
	hr = getEngine()->getDevice()->CreateInputLayout(
		GMSHADER_ElementDescriptions,
		GM_array_size(GMSHADER_ElementDescriptions),
		getEngine()->getVertexShaderBuffer()->GetBufferPointer(),
		getEngine()->getVertexShaderBuffer()->GetBufferSize(),
		&d->inputLayout
	);
	GM_COM_CHECK(hr);
}

void GMDx11Renderer::beginModel(GMModel* model, const GMGameObject* parent)
{
	D(d);
	// Renderer决定自己的顶点Layout
	ID3D11DeviceContext* context = getEngine()->getDeviceContext();
	context->IASetInputLayout(d->inputLayout);
	context->IASetPrimitiveTopology(getMode(model->getMesh()));
	context->VSSetShader(getEngine()->getVertexShader(), NULL, 0);
	context->PSSetShader(getEngine()->getPixelShader(), NULL, 0);
	GM.getCamera().getFrustum().setDxModelMatrix(parent->getTransform());
}

void GMDx11Renderer::endModel()
{
}

void GMDx11Renderer::draw(GMComponent* component, GMMesh* mesh)
{
	D(d);
	ID3D11DeviceContext* context = getEngine()->getDeviceContext();
	GMDx11RasterStates& rasterStates = GMDx11RasterStates::instance();

	d->shader = &component->getShader();
	rasterStates.applyRasterStates(
		d->shader->getCull(),
		d->shader->getFrontFace()
	);
	getEngine()->getDeviceContext()->Draw(3, 0);
	//TODO 是否要恢复RasterStates?
}