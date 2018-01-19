#include "stdafx.h"
#include "gmmodel.h"
#include <linearmath.h>
#include <algorithm>
#include <iterator>
#include "foundation/gamemachine.h"

#define VERTEX_OFFSET(offset, idx) ((offset * GMModel::PositionDimension) + idx)
#define UV_OFFSET(offset, idx) ((offset << 1) + idx)

GMModel::GMModel()
{
	D(d);
	d->mesh = new GMMesh();
}

GMModel::GMModel(GMModel& model)
{
	D(d);
	GMMesh* mesh = model.getMesh();

	if (!model.getPainter())
		GM.createModelPainterAndTransfer(&model);
	needNotTransferAnymore(); // 不需要再将顶点数据传输到显卡了

	d->mesh = new GMMesh();
	d->mesh->setMeshData(mesh->getMeshData());
	d->mesh->setArrangementMode(mesh->getArrangementMode());
	d->mesh->setName(mesh->getName());
	GM_FOREACH_ENUM_CLASS(dt, GMVertexDataType::Position, GMVertexDataType::EndOfVertexDataType)
	{
		if (mesh->isDataDisabled(dt))
			d->mesh->disableData(dt);
	}

	GM_COPY_VERTEX_PROPERTY(d->mesh, mesh, positions);
	GM_COPY_VERTEX_PROPERTY(d->mesh, mesh, normals);
	GM_COPY_VERTEX_PROPERTY(d->mesh, mesh, uvs);
	GM_COPY_VERTEX_PROPERTY(d->mesh, mesh, tangents);
	GM_COPY_VERTEX_PROPERTY(d->mesh, mesh, bitangents);
	GM_COPY_VERTEX_PROPERTY(d->mesh, mesh, lightmaps);
	GM_COPY_VERTEX_PROPERTY(d->mesh, mesh, colors);

	// 开始拷贝components中的shaders部分
	auto& srcComponents = model.getMesh()->getComponents();
	for (auto& components : srcComponents)
	{
		GMComponent* c = new GMComponent(d->mesh);
		*c = *components;
		c->setParentMesh(d->mesh); //将mesh设置回来
	}
}

GMModel::~GMModel()
{
	D(d);
	GM_delete(d->mesh);
}

void GMModel::releaseMesh()
{
	D(d);
	d->mesh->releaseMeshData();
}

GMMeshData::GMMeshData()
{
	D(d);
	d->ref = 1;
	GM.getFactory()->createPainter(nullptr, nullptr, &d->painter);
}

GMMeshData::~GMMeshData()
{
	D(d);
	GM_delete(d->painter);
}

void GMMeshData::dispose()
{
	D(d);
	d->painter->dispose(this);
}

GMComponent::GMComponent(GMMesh* parent)
{
	D(d);
	d->parentMesh = parent;
	setVertexOffset(d->parentMesh->positions().size() / GMModel::PositionDimension);
	d->parentMesh->appendComponent(this);
}

void GMComponent::clear()
{
	D(d);
	d->primitiveCount = 0;
	d->primitiveVertices.clear();
	d->vertexOffsets.clear();
}

void GMComponent::setVertexOffset(GMuint offset)
{
	D(d);
	d->offset = offset;
}

void GMComponent::beginFace()
{
	D(d);
	d->currentFaceVerticesCount = 0;
}

void GMComponent::vertex(GMfloat x, GMfloat y, GMfloat z)
{
	D(d);
	auto& vertices = d->parentMesh->positions();
	vertices.push_back(x);
	vertices.push_back(y);
	vertices.push_back(z);
	d->currentFaceVerticesCount++;
}

void GMComponent::normal(GMfloat x, GMfloat y, GMfloat z)
{
	D(d);
	auto& normals = d->parentMesh->normals();
	normals.push_back(x);
	normals.push_back(y);
	normals.push_back(z);
}

void GMComponent::uv(GMfloat u, GMfloat v)
{
	D(d);
	auto& uvs = d->parentMesh->uvs();
	uvs.push_back(u);
	uvs.push_back(v);
}

void GMComponent::lightmap(GMfloat u, GMfloat v)
{
	D(d);
	auto& lightmaps = d->parentMesh->lightmaps();
	lightmaps.push_back(u);
	lightmaps.push_back(v);
}

void GMComponent::color(GMfloat r, GMfloat g, GMfloat b, GMfloat a)
{
	D(d);
	auto& colors = d->parentMesh->colors();
	colors.push_back(r);
	colors.push_back(g);
	colors.push_back(b);
	colors.push_back(a);
}

void GMComponent::endFace()
{
	D(d);
	d->vertexOffsets.push_back(d->primitiveVertices.empty() ?
		d->offset : d->vertexOffsets.back() + d->primitiveVertices.back()
	);
	d->primitiveVertices.push_back(d->currentFaceVerticesCount);
	d->primitiveCount++;
}

// 复制count倍当前component
void GMComponent::expand(GMuint count)
{
	D(d);
	IF_ENABLED(d->parentMesh, GMVertexDataType::Position)	d->parentMesh->positions().reserve(d->parentMesh->positions().size() * count);
	IF_ENABLED(d->parentMesh, GMVertexDataType::Normal)		d->parentMesh->normals().reserve(d->parentMesh->normals().size() * count);
	IF_ENABLED(d->parentMesh, GMVertexDataType::UV)			d->parentMesh->uvs().reserve(d->parentMesh->uvs().size() * count);
	IF_ENABLED(d->parentMesh, GMVertexDataType::Tangent)	d->parentMesh->tangents().reserve(d->parentMesh->tangents().size() * count);
	IF_ENABLED(d->parentMesh, GMVertexDataType::Bitangent)	d->parentMesh->bitangents().reserve(d->parentMesh->bitangents().size() * count);
	IF_ENABLED(d->parentMesh, GMVertexDataType::Lightmap)	d->parentMesh->lightmaps().reserve(d->parentMesh->lightmaps().size() * count);
	IF_ENABLED(d->parentMesh, GMVertexDataType::Color)		d->parentMesh->colors().reserve(d->parentMesh->colors().size() * count);

	GMint verticesCount = 0;
	Vector<GMint> flag;
	for (auto& c : d->primitiveVertices)
	{
		verticesCount += c;
		flag.push_back(verticesCount);
	}

	for (GMuint turn = 0; turn < count - 1; turn++)
	{
		for (GMint i = 0; i < verticesCount; i++)
		{
			if (i == 0)
				beginFace();
			if (std::find(flag.begin(), flag.end(), i) != flag.end())
			{
				endFace();
				beginFace();
			}

			IF_ENABLED(d->parentMesh, GMVertexDataType::Position)	vertex(d->parentMesh->positions()[d->offset + i * GMModel::PositionDimension + 0], d->parentMesh->positions()[d->offset + i * GMModel::PositionDimension + 1], d->parentMesh->positions()[d->offset + i * GMModel::PositionDimension + 2]);
			IF_ENABLED(d->parentMesh, GMVertexDataType::Normal)		normal(d->parentMesh->normals()[d->offset + i * GMModel::NormalDimension + 0], d->parentMesh->normals()[d->offset + i * GMModel::NormalDimension + 1], d->parentMesh->normals()[d->offset + i * GMModel::NormalDimension + 2]);
			IF_ENABLED(d->parentMesh, GMVertexDataType::UV)			uv(d->parentMesh->uvs()[d->offset + i * GMModel::UVDimension + 0], d->parentMesh->uvs()[d->offset + i * GMModel::UVDimension + 1]);
			IF_ENABLED(d->parentMesh, GMVertexDataType::Lightmap)	lightmap(d->parentMesh->lightmaps()[d->offset + i * GMModel::TextureDimension + 0], d->parentMesh->lightmaps()[d->offset + i * GMModel::TextureDimension + 1]);
			IF_ENABLED(d->parentMesh, GMVertexDataType::Color)		color(d->parentMesh->colors()[d->offset + i * GMModel::TextureDimension + 0], d->parentMesh->colors()[d->offset + i * GMModel::TextureDimension + 1], d->parentMesh->colors()[d->offset + i * GMModel::TextureDimension + 2]);

			if (i == verticesCount - 1)
				endFace();
		}
	}
}

GMMesh::GMMesh()
{
	D(d);
	d->meshData = new GMMeshData();
}

GMMesh::~GMMesh()
{
	D(d);
	releaseMeshData();
	for (auto component : d->components)
	{
		GM_delete(component);
	}
}

void GMMesh::appendComponent(AUTORELEASE GMComponent* component)
{
	D(d);
	GM_ASSERT(std::find(d->components.begin(), d->components.end(), component) == d->components.end());
	d->components.push_back(component);
}

void GMMesh::calculateTangentSpace()
{
	D(d);
	if (d->uvs.size() == 0)
		return;

	for (auto component : d->components)
	{
		for (GMuint i = 0; i < component->getPrimitiveCount(); i++)
		{
			// 每一个多边形拥有一个切线空间，这意味着多于3个点的多边形需要共面，否则切线空间会有问题
			GMint o = component->getOffsetPtr()[i];

			glm::vec3 e0(d->positions[VERTEX_OFFSET(o, 0)], d->positions[VERTEX_OFFSET(o, 1)], d->positions[VERTEX_OFFSET(o, 2)]);
			glm::vec3 e1(d->positions[VERTEX_OFFSET(o, 3)], d->positions[VERTEX_OFFSET(o, 4)], d->positions[VERTEX_OFFSET(o, 5)]);
			glm::vec3 e2(d->positions[VERTEX_OFFSET(o, 6)], d->positions[VERTEX_OFFSET(o, 7)], d->positions[VERTEX_OFFSET(o, 8)]);

			glm::vec2 uv0(d->uvs[UV_OFFSET(o, 0)], d->uvs[UV_OFFSET(o, 1)]);
			glm::vec2 uv1(d->uvs[UV_OFFSET(o, 2)], d->uvs[UV_OFFSET(o, 3)]);
			glm::vec2 uv2(d->uvs[UV_OFFSET(o, 4)], d->uvs[UV_OFFSET(o, 5)]);

			glm::vec3 E1 = e1 - e0;
			glm::vec3 E2 = e2 - e0;
			glm::vec2 deltaUV1 = uv1 - uv0;
			glm::vec2 deltaUV2 = uv2 - uv0;

			GMfloat s = 1.0f / (deltaUV1[0] * deltaUV2[1] - deltaUV1[1] * deltaUV2[0]);

			GMfloat tangents[3] = {
				s * (deltaUV2[1] * E1[0] - deltaUV1[1] * E2[0]),
				s * (deltaUV2[1] * E1[1] - deltaUV1[1] * E2[1]),
				s * (deltaUV2[1] * E1[2] - deltaUV1[1] * E2[2])
			};
			GMfloat bitangents[3] = {
				s * (deltaUV1[0] * E2[0] - deltaUV2[0] * E1[0]),
				s * (deltaUV1[0] * E2[1] - deltaUV2[0] * E1[1]),
				s * (deltaUV1[0] * E2[2] - deltaUV2[0] * E1[2])
			};

			glm::vec3 tangentVector = glm::fastNormalize(glm::vec3(tangents[0], tangents[1], tangents[2]));
			glm::vec3 bitangentVector = glm::fastNormalize(glm::vec3(bitangents[0], bitangents[1], bitangents[2]));

			GMint verticesCount = component->getPrimitiveVerticesCountPtr()[i];
			for (GMint j = 0; j < verticesCount; j++)
			{
				d->tangents.push_back(tangentVector[0]);
				d->tangents.push_back(tangentVector[1]);
				d->tangents.push_back(tangentVector[2]);

				d->bitangents.push_back(bitangentVector[0]);
				d->bitangents.push_back(bitangentVector[1]);
				d->bitangents.push_back(bitangentVector[2]);
			}
		}
	}
}

void GMMesh::releaseMeshData()
{
	D(d);
	d->meshData->releaseRef();
	if (d->meshData->hasNoRef())
		GM_delete(d->meshData);
}

void GMMesh::setMeshData(GMMeshData* md)
{
	D(d);
	if (d->meshData != md)
	{
		GM_delete(d->meshData);
		d->meshData = md;
		md->addRef();
	}
}