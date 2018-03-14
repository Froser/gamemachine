#include "stdafx.h"
#include "gmdx11modelpainter.h"
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"

GMDx11ModelPainter::GMDx11ModelPainter(GMDx11GraphicEngine* engine, GMModel* model)
	: GMModelPainter(model)
{
	D(d);
	d->engine = engine;

	// 初始化一个数组，用于遍历顶点数据
	if (model)
	{
		GMMesh* mesh = model->getMesh();
		d->vertexData[(size_t)GMVertexDataType::EndOfVertexDataType];
		d->vertexData[(size_t)GMVertexDataType::Position] = &mesh->positions();
		d->vertexData[(size_t)GMVertexDataType::Normal] = &mesh->normals();
		d->vertexData[(size_t)GMVertexDataType::UV] = &mesh->uvs();
		d->vertexData[(size_t)GMVertexDataType::Tangent] = &mesh->tangents();
		d->vertexData[(size_t)GMVertexDataType::Bitangent] = &mesh->bitangents();
		d->vertexData[(size_t)GMVertexDataType::Lightmap] = &mesh->lightmaps();
		d->vertexData[(size_t)GMVertexDataType::Color] = &mesh->colors();
	}
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

	HRESULT hr;
	GM_FOREACH_ENUM_CLASS(type, GMVertexDataType::Position, GMVertexDataType::EndOfVertexDataType)
	{
		size_t byteWidth = mesh->isDataDisabled(type) ? 0 : sizeof(GMModel::DataType) * d->vertexData[(size_t)type]->size();

		if (byteWidth == 0)
		{
			// 用0填充不存在的数据段 TODO 是否有更好顶点结构？
			GM_ASSERT(mesh->positions().size() > 0);
			d->vertexData[(size_t)type]->resize(mesh->positions().size());
			byteWidth = d->vertexData[(size_t)type]->size();
		}

		const int& idx = (size_t)type;
		D3D11_BUFFER_DESC bufDesc;
 		bufDesc.Usage = usage;
		bufDesc.ByteWidth = byteWidth;
		bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufDesc.CPUAccessFlags = 0;
		bufDesc.MiscFlags = 0;
		bufDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA bufData;
		bufData.pSysMem = d->vertexData[(size_t)type]->data();
		bufData.SysMemPitch = bufData.SysMemSlicePitch = 0;

		GMComPtr<ID3D11Device> device = d->engine->getDevice();
		hr = device->CreateBuffer(&bufDesc, &bufData, &d->buffers[idx]);
		GM_COM_CHECK(hr);

		break; //TODO TEST
	}

	d->inited = true;
	model->needNotTransferAnymore();
}

void GMDx11ModelPainter::draw(const GMGameObject* parent)
{
	D(d);
	GMModel* model = getModel();
	IRenderer* renderer = d->engine->getRenderer(model->getType());
	renderer->beginModel(model, parent);

	GMMesh* mesh = getModel()->getMesh();
	for (auto component : mesh->getComponents())
	{
		GMShader& shader = component->getShader();
		if (shader.getNodraw())
			continue;

		draw(renderer, component, mesh);
	}
	renderer->endModel();
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

void GMDx11ModelPainter::draw(IRenderer* renderer, GMComponent* component, GMMesh* mesh)
{
	D(d);
	// TODO 是否在这里设置？应该全部给render来做
	GMuint strides[(size_t)GMVertexDataType::EndOfVertexDataType] = { 3 * sizeof(GMfloat) };
	GMuint offsets[(size_t)GMVertexDataType::EndOfVertexDataType] = { 0 };
	// context->IASetVertexBuffers(0, (size_t)GMVertexDataType::EndOfVertexDataType, &d->buffers[0], strides, offsets);
	d->engine->getDeviceContext()->IASetVertexBuffers(0, 1, &d->buffers[0], strides, offsets);

	renderer->draw(component, mesh);
}