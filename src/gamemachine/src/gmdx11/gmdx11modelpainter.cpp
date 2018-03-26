#include "stdafx.h"
#include "gmdx11modelpainter.h"
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"

#define SAFE_ASSIGN(mb, df, pos) ((pos) >= (mb).size()) ? df : (mb[pos])

GMDx11ModelPainter::GMDx11ModelPainter(GMDx11GraphicEngine* engine, GMModel* model)
	: GMModelPainter(model)
{
	D(d);
	d->engine = engine;
}

bool GMDx11ModelPainter::getInterface(GameMachineInterfaceID id, void** out)
{
	D(d);
	if (id == GameMachineInterfaceID::D3D11Buffer)
	{
		d->buffer->AddRef();
		(*out) = d->buffer;
		return true;
	}
	return false;
}

void GMDx11ModelPainter::packData(Vector<GMDx11VertexData>& packedData)
{
	D(d);
	GMDx11VertexData vd = { 0 };
	GMModel* model = getModel();
	// 按照position的size()/3来分配顶点
	GM_ASSERT(model->getMesh()->positions().size() % 3 == 0);
	for (GMuint i = 0; i < model->getMesh()->positions().size() / 3; ++i)
	{
		{
			auto& data_ref = model->getMesh()->positions();
			vd.vertices[0] = data_ref[i * 3];
			vd.vertices[1] = data_ref[i * 3 + 1];
			vd.vertices[2] = data_ref[i * 3 + 2];
		}

		{
			auto& data_ref = model->getMesh()->normals();
			vd.normals[0] = SAFE_ASSIGN(data_ref, 0, i * 3);
			vd.normals[1] = SAFE_ASSIGN(data_ref, 0, i * 3 + 1);
			vd.normals[2] = SAFE_ASSIGN(data_ref, 0, i * 3 + 2);
		}

		{
			auto& data_ref = model->getMesh()->texcoords();
			vd.texcoords[0] = SAFE_ASSIGN(data_ref, 0, i * 2);
			vd.texcoords[1] = SAFE_ASSIGN(data_ref, 0, i * 2 + 1);
		}

		{
			auto& data_ref = model->getMesh()->tangents();
			vd.tangents[0] = SAFE_ASSIGN(data_ref, 0, i * 2);
			vd.tangents[1] = SAFE_ASSIGN(data_ref, 0, i * 2 + 1);
		}

		{
			auto& data_ref = model->getMesh()->bitangents();
			vd.bitangents[0] = SAFE_ASSIGN(data_ref, 0, i * 2);
			vd.bitangents[1] = SAFE_ASSIGN(data_ref, 0, i * 2 + 1);
		}

		packedData.push_back(vd);
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

	Vector<GMDx11VertexData> packedData;
	// 把数据打入顶点数组
	packData(packedData);

	D3D11_USAGE usage = model->getUsageHint() == GMUsageHint::StaticDraw ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;
	D3D11_BUFFER_DESC bufDesc;
 	bufDesc.Usage = usage;
	bufDesc.ByteWidth = packedData.size() * sizeof(decltype(packedData)::value_type);
	bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufDesc.CPUAccessFlags = 0;
	bufDesc.MiscFlags = 0;
	bufDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA bufData;
	bufData.pSysMem = packedData.data();
	bufData.SysMemPitch = bufData.SysMemSlicePitch = 0;

	GMComPtr<ID3D11Device> device = d->engine->getDevice();
	GM_DX_HR(device->CreateBuffer(&bufDesc, &bufData, &d->buffer));

	IF_ENABLED(mesh, GMVertexDataType::Position)	mesh->clear_positions_and_save_byte_size();
	IF_ENABLED(mesh, GMVertexDataType::Normal)		mesh->clear_normals_and_save_byte_size();
	IF_ENABLED(mesh, GMVertexDataType::UV)			mesh->clear_texcoords_and_save_byte_size();
	IF_ENABLED(mesh, GMVertexDataType::Tangent)		mesh->clear_tangents_and_save_byte_size();
	IF_ENABLED(mesh, GMVertexDataType::Bitangent)	mesh->clear_bitangents_and_save_byte_size();
	IF_ENABLED(mesh, GMVertexDataType::Lightmap)	mesh->clear_lightmaps_and_save_byte_size();
	IF_ENABLED(mesh, GMVertexDataType::Color)		mesh->clear_colors_and_save_byte_size();

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
	renderer->draw(this, component, mesh);
}