#include "stdafx.h"
#include "gmdx11modelpainter.h"
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"

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
}

GMDx11ModelPainter::GMDx11ModelPainter(GMDx11GraphicEngine* engine, GMModel* model)
	: GMModelPainter(model)
{
	D(d);
	d->engine = engine;
}

void GMDx11ModelPainter::transfer()
{
	D(d);
	if (d->inited)
		return;

	GMModel* model = getModel();
	if (!model->isNeedTransfer())
		return;

	GMMesh* mesh = model->getMesh();
	mesh->calculateTangentSpace();
	
	D3D11_USAGE usage = model->getUsageHint() == GMUsageHint::StaticDraw ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;

	size_t positionSize = mesh->isDataDisabled(GMVertexDataType::Position) ? 0 : sizeof(GMModel::DataType) * mesh->positions().size();
	size_t normalSize = mesh->isDataDisabled(GMVertexDataType::Normal) ? 0 : sizeof(GMModel::DataType) * mesh->normals().size();
	size_t uvSize = mesh->isDataDisabled(GMVertexDataType::UV) ? 0 : sizeof(GMModel::DataType) * mesh->uvs().size();
	size_t tangentSize = mesh->isDataDisabled(GMVertexDataType::Tangent) ? 0 : sizeof(GMModel::DataType) * mesh->tangents().size();
	size_t bitangentSize = mesh->isDataDisabled(GMVertexDataType::Bitangent) ? 0 : sizeof(GMModel::DataType) * mesh->bitangents().size();
	size_t lightmapSize = mesh->isDataDisabled(GMVertexDataType::Lightmap) ? 0 : sizeof(GMModel::DataType) * mesh->lightmaps().size();
	size_t colorSize = mesh->isDataDisabled(GMVertexDataType::Color) ? 0 : sizeof(GMModel::DataType) * mesh->colors().size();

	D3D11_BUFFER_DESC bufDesc;
	bufDesc.Usage = D3D11_USAGE_DEFAULT;
	bufDesc.ByteWidth = positionSize + normalSize + uvSize + tangentSize + bitangentSize + lightmapSize + colorSize;
	bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufDesc.CPUAccessFlags = 0;
	bufDesc.MiscFlags = 0;
	bufDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA bufData;
	//TODO 应该放入所有数据
	bufData.pSysMem = mesh->positions().data();
	bufData.SysMemPitch = bufData.SysMemSlicePitch = 0;

	HRESULT hr;
	GMComPtr<ID3D11Device> device = d->engine->getDevice();
	hr = device->CreateBuffer(&bufDesc, &bufData, &d->buffer);
	GM_COM_CHECK(hr);

	hr = renderBuffer();
	GM_COM_CHECK(hr);

	d->inited = true;
	model->needNotTransferAnymore();
}

void GMDx11ModelPainter::draw(const GMGameObject* parent)
{

	throw std::logic_error("The method or operation is not implemented.");
}

void GMDx11ModelPainter::dispose(GMMeshData* md)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void GMDx11ModelPainter::beginUpdateBuffer(GMMesh* mesh)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void GMDx11ModelPainter::endUpdateBuffer()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void* GMDx11ModelPainter::getBuffer()
{
	throw std::logic_error("The method or operation is not implemented.");
}

HRESULT GMDx11ModelPainter::renderBuffer()
{
	D(d);
	//TODO 应该放入所有缓存
	ID3D11DeviceContext* context = d->engine->getDeviceContext();
	GMuint stride = sizeof(gm::GMVertexDataType);
	context->IASetVertexBuffers(0, 1, &d->buffer, &stride, 0);
	context->IASetPrimitiveTopology(getMode(getModel()->getMesh()));
	return S_OK;
}