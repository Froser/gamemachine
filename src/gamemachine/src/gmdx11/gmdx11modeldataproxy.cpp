#include "stdafx.h"
#include "gmdx11modeldataproxy.h"
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"

GMDx11ModelDataProxy::GMDx11ModelDataProxy(const IRenderContext* context, GMModel* model)
	: GMModelDataProxy(context, model)
{
	D(d);
	if (context)
		d->engine = gm_cast<GMDx11GraphicEngine*>(context->getEngine());
}

GMDx11ModelDataProxy::~GMDx11ModelDataProxy()
{
}

bool GMDx11ModelDataProxy::getInterface(GameMachineInterfaceID id, void** out)
{
	D_BASE(db, Base);
	D(d);
	if (id == GameMachineInterfaceID::D3D11VertexBuffer)
	{
		if (d->vertexBuffer)
		{
			d->vertexBuffer->AddRef();
			(*out) = d->vertexBuffer;
		}
		else
		{
			ID3D11Buffer* buffer = db->model->getModelBuffer()->getMeshBuffer().vertexBuffer;
			GM_ASSERT(buffer);
			buffer->AddRef();
			(*out) = buffer;
		}
		return true;
	}
	else if (id == GameMachineInterfaceID::D3D11IndexBuffer)
	{
		if (d->indexBuffer)
		{
			d->indexBuffer->AddRef();
			(*out) = d->indexBuffer;
		}
		else
		{
			ID3D11Buffer* buffer = db->model->getModelBuffer()->getMeshBuffer().indexBuffer;
			GM_ASSERT(buffer);
			buffer->AddRef();
			(*out) = buffer;
		}
		return true;
	}
	return false;
}

void GMDx11ModelDataProxy::transfer()
{
	D(d);
	D_BASE(db, GMModelDataProxy);
	if (d->inited)
		return;

	prepareParentModel();

	GMModel* model = getModel();
	if (!model->isNeedTransfer())
		return;

	prepareTangentSpace();

	Vector<GMVertex> packedVertices;
	// 把数据打入顶点数组
	packVertices(packedVertices);
	GMsize_t verticesCount = 0;

	D3D11_USAGE usage = model->getUsageHint() == GMUsageHint::StaticDraw ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;
	D3D11_BUFFER_DESC bufDesc;
	bufDesc.Usage = usage;
	GM_ASSERT(packedVertices.size() * sizeof(decltype(packedVertices)::value_type) < std::numeric_limits<UINT>::max());
	bufDesc.ByteWidth = (UINT)(packedVertices.size() * sizeof(decltype(packedVertices)::value_type));
	bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufDesc.CPUAccessFlags = usage == D3D11_USAGE_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;
	bufDesc.MiscFlags = 0;
	bufDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA bufData;
	bufData.pSysMem = packedVertices.data();
	bufData.SysMemPitch = bufData.SysMemSlicePitch = 0;

	ID3D11Device* device = d->engine->getDevice();
	GM_DX_HR(device->CreateBuffer(&bufDesc, &bufData, &d->vertexBuffer));

	if (model->getDrawMode() == GMModelDrawMode::Index)
	{
		Vector<GMuint32> packedIndices;
		// 把数据打入顶点数组
		packIndices(packedIndices);

		// 如果是索引缓存，需要构建一份索引数据
		D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
		D3D11_BUFFER_DESC bufDesc;
		bufDesc.Usage = usage;
		bufDesc.ByteWidth = gm_sizet_to<UINT>(packedIndices.size() * sizeof(decltype(packedIndices)::value_type));
		bufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufDesc.CPUAccessFlags = 0;
		bufDesc.MiscFlags = 0;
		bufDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA bufData;
		bufData.pSysMem = packedIndices.data();
		bufData.SysMemPitch = bufData.SysMemSlicePitch = 0;

		GMComPtr<ID3D11Device> device = d->engine->getDevice();
		GM_DX_HR(device->CreateBuffer(&bufDesc, &bufData, &d->indexBuffer));

		verticesCount = packedIndices.size();
	}
	else
	{
		verticesCount = packedVertices.size();
	}

	GMModelBufferData modelBufferData;
	modelBufferData.vertexBuffer = d->vertexBuffer;
	modelBufferData.indexBuffer = d->indexBuffer;
	GMModelBuffer* mb = new GMModelBuffer();
	mb->setData(modelBufferData);
	model->setModelBuffer(mb);
	mb->releaseRef();
	model->setVerticesCount(verticesCount);

	for (auto& part : model->getParts())
	{
		part->clear();
	}

	d->inited = true;
	model->doNotTransferAnymore();
}

void GMDx11ModelDataProxy::dispose(GMModelBuffer* md)
{
}

void GMDx11ModelDataProxy::beginUpdateBuffer(GMModelBufferType type)
{
	D(d);
	// 不能在多线程中，或者嵌套中操作同一个Buffer
	GM_ASSERT(!d->mappedSubResource);
	d->mappedSubResource = new D3D11_MAPPED_SUBRESOURCE();
	ID3D11DeviceContext* context = d->engine->getDeviceContext();
	d->lastType = type;
	if (type == GMModelBufferType::VertexBuffer)
	{
		GM_DX_HR(context->Map(d->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, d->mappedSubResource));
	}
	else
	{
		GM_DX_HR(context->Map(d->indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, d->mappedSubResource));
	}
}

void GMDx11ModelDataProxy::endUpdateBuffer()
{
	D(d);
	GM_ASSERT(d->mappedSubResource);
	ID3D11DeviceContext* context = d->engine->getDeviceContext();
	if (d->lastType == GMModelBufferType::VertexBuffer)
		context->Unmap(d->vertexBuffer, 0);
	else
		context->Unmap(d->indexBuffer, 0);

	GM_delete(d->mappedSubResource);
	d->mappedSubResource = nullptr;
}

void* GMDx11ModelDataProxy::getBuffer()
{
	D(d);
	if (!d->mappedSubResource)
		return nullptr;

	return d->mappedSubResource->pData;
}
