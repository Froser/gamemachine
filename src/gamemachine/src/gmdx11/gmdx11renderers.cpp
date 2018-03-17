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
			bool multisampleEnable = GM.getGameMachineRunningStates().sampleCount > 1;
			if (!states[(GMuint)cullMode][(GMuint)frontFace])
			{
				D3D11_RASTERIZER_DESC desc = GMGetDefaultRasterizerDesc(multisampleEnable, multisampleEnable);
				desc.CullMode = cullMode == GMS_Cull::CULL ? D3D11_CULL_BACK : D3D11_CULL_NONE;
				desc.FrontCounterClockwise = frontFace == GMS_FrontFace::CLOCKWISE ? FALSE : TRUE;
				createRasterizerState(desc, &states[(GMuint)cullMode][(GMuint)frontFace]);
			}

			GM_ASSERT(states[(GMuint)cullMode][(GMuint)frontFace]);
			engine->getDeviceContext()->RSSetState(states[(GMuint)cullMode][(GMuint)frontFace]);
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
	// 定义顶点布局
	GM_ASSERT(!d->inputLayout);
	getEngine()->createInputLayout(
		GMSHADER_ElementDescriptions,
		GM_array_size(GMSHADER_ElementDescriptions),
		&d->inputLayout
	);
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
	GMDx11RasterStates& rasterStates = GMDx11RasterStates::instance();
	rasterStates.applyRasterStates(
		d->shader->getCull(),
		d->shader->getFrontFace()
	);
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

	prepareTextures();
	prepareBuffer(painter);
	prepareRasterizer(component);

	GMuint primitiveCount = component->getPrimitiveCount();
	GMuint* offsetPtr = component->getOffsetPtr();
	GMuint* vertexCountPtr = component->getPrimitiveVerticesCountPtr();
	for (GMuint i = 0; i < primitiveCount; ++i)
	{
		getEngine()->getDeviceContext()->Draw(vertexCountPtr[i], offsetPtr[i]);
	}
}