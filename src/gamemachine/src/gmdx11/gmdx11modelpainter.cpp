#include "stdafx.h"
#include "gmdx11modelpainter.h"
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"

GMDx11ModelPainter::GMDx11ModelPainter(GMDx11GraphicEngine* engine, GMModel* model)
	: GMModelPainter(model)
{
	D(d);
	d->engine = engine;
}

bool GMDx11ModelPainter::getInterface(GameMachineInterfaceID id, void** out)
{
	D_BASE(db, Base);
	D(d);
	if (id == GameMachineInterfaceID::D3D11Buffer)
	{
		if (d->buffer)
		{
			d->buffer->AddRef();
			(*out) = d->buffer;
		}
		else
		{
			ID3D11Buffer* buffer = reinterpret_cast<ID3D11Buffer*>(db->model->getBuffer()->buffer);
			GM_ASSERT(buffer);
			buffer->AddRef();
			(*out) = buffer;
		}
		return true;
	}
	return false;
}

void GMDx11ModelPainter::transfer()
{
	D(d);
	if (d->inited)
		return;

	GMModel* model = getModel();
	if (!model->isNeedTransfer())
		return;

	for (auto& mesh : model->getMeshes())
	{
		mesh->calculateTangentSpace();
	}

	Vector<GMVertex> packedData;
	// 把数据打入顶点数组
	packData(packedData);

	D3D11_USAGE usage = model->getUsageHint() == GMUsageHint::StaticDraw ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;
	D3D11_BUFFER_DESC bufDesc;
 	bufDesc.Usage = usage;
	bufDesc.ByteWidth = packedData.size() * sizeof(decltype(packedData)::value_type);
	bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufDesc.CPUAccessFlags = usage == D3D11_USAGE_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;
	bufDesc.MiscFlags = 0;
	bufDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA bufData;
	bufData.pSysMem = packedData.data();
	bufData.SysMemPitch = bufData.SysMemSlicePitch = 0;

	GMComPtr<ID3D11Device> device = d->engine->getDevice();
	GM_DX_HR(device->CreateBuffer(&bufDesc, &bufData, &d->buffer));

	GMModelBufferData modelBufferData;
	modelBufferData.buffer = d->buffer;
	GMModelBuffer* mb = new GMModelBuffer();
	mb->setData(modelBufferData);
	model->setModelBuffer(mb);
	model->setVerticesCount(packedData.size());

	for (auto& mesh : model->getMeshes())
	{
		mesh->clear();
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

	if (model->getShader().getDiscard())
		return;

	draw(renderer, model);
	renderer->endModel();
}

void GMDx11ModelPainter::dispose(GMModelBuffer* md)
{
}

void GMDx11ModelPainter::beginUpdateBuffer(GMModel* model)
{
	D(d);
	// 不能在多线程中，或者嵌套中操作同一个Buffer
	GM_ASSERT(!d->mappedSubResource);
	d->mappedSubResource = new D3D11_MAPPED_SUBRESOURCE();
	ID3D11DeviceContext* context = d->engine->getDeviceContext();
	GM_DX_HR(context->Map(d->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, d->mappedSubResource));
}

void GMDx11ModelPainter::endUpdateBuffer()
{
	D(d);
	GM_ASSERT(d->mappedSubResource);
	ID3D11DeviceContext* context = d->engine->getDeviceContext();
	context->Unmap(d->buffer, 0);

	GM_delete(d->mappedSubResource);
	d->mappedSubResource = nullptr;
}

void* GMDx11ModelPainter::getBuffer()
{
	D(d);
	if (!d->mappedSubResource)
		return nullptr;

	return d->mappedSubResource->pData;
}

void GMDx11ModelPainter::draw(IRenderer* renderer, GMModel* model)
{
	renderer->draw(this, model);
}