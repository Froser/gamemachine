#include "stdafx.h"
#include "object.h"
#include "foundation/linearmath.h"
#include <algorithm>

#define VERTEX_DEMENSION 4 //顶点的维度，最高维度是齐次维度，恒为1
#define VERTEX_OFFSET(offset, idx) ((offset * VERTEX_DEMENSION) + idx)
#define UV_OFFSET(offset, idx) ((offset << 1) + idx)

Object* GMObjectPainter::getObject()
{
	D(d);
	return d->object;
}

Object::~Object()
{
	D(d);
	if (d->painter)
		d->painter->dispose();

	BEGIN_FOREACH_OBJ(this, meshes)
	{
		if (meshes)
			delete meshes;
	}
	END_FOREACH_OBJ
}

Component::Component(GMMesh* parent)
{
	D(d);
	d->parentMesh = parent;
	setVertexOffset(d->parentMesh->positions().size() / VERTEX_DEMENSION);
}

// 设置此component的第一个顶点位于ChildObject.vertices()中的偏移位置
// 一般不需要手动调用
void Component::setVertexOffset(GMuint offset)
{
	D(d);
	d->offset = offset;
}

void Component::beginFace()
{
	D(d);
	d->currentFaceVerticesCount = 0;
}

void Component::vertex(GMfloat x, GMfloat y, GMfloat z)
{
	D(d);
	AlignedVector<Object::DataType>& vertices = d->parentMesh->positions();
	vertices.push_back(x);
	vertices.push_back(y);
	vertices.push_back(z);
	vertices.push_back(1.0f);
	d->currentFaceVerticesCount++;
}

void Component::normal(GMfloat x, GMfloat y, GMfloat z)
{
	D(d);
	AlignedVector<Object::DataType>& normals = d->parentMesh->normals();
	normals.push_back(x);
	normals.push_back(y);
	normals.push_back(z);
	normals.push_back(1.0f);
}

void Component::uv(GMfloat u, GMfloat v)
{
	D(d);
	AlignedVector<Object::DataType>& uvs = d->parentMesh->uvs();
	uvs.push_back(u);
	uvs.push_back(v);
}

void Component::lightmap(GMfloat u, GMfloat v)
{
	D(d);
	AlignedVector<Object::DataType>& lightmaps = d->parentMesh->lightmaps();
	lightmaps.push_back(u);
	lightmaps.push_back(v);
}

void Component::endFace()
{
	D(d);
	d->vertexOffsets.push_back(d->primitiveVertices.empty() ?
		d->offset : d->vertexOffsets.back() + d->primitiveVertices.back()
	);
	d->primitiveVertices.push_back(d->currentFaceVerticesCount);
	d->primitiveCount++;
}

GMMesh::GMMesh()
{
}

GMMesh::~GMMesh()
{
	D(d);
	for (auto component : d->components)
	{
		delete component;
	}
}

void GMMesh::appendComponent(AUTORELEASE Component* component)
{
	D(d);
	ASSERT(d->components.find(component) == d->components.end());
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

			linear_math::Vector3 e0(d->positions[VERTEX_OFFSET(o, 0)], d->positions[VERTEX_OFFSET(o, 1)], d->positions[VERTEX_OFFSET(o, 2)]);
			linear_math::Vector3 e1(d->positions[VERTEX_OFFSET(o, 4)], d->positions[VERTEX_OFFSET(o, 5)], d->positions[VERTEX_OFFSET(o, 6)]);
			linear_math::Vector3 e2(d->positions[VERTEX_OFFSET(o, 8)], d->positions[VERTEX_OFFSET(o, 9)], d->positions[VERTEX_OFFSET(o, 10)]);

			linear_math::Vector2 uv0(d->uvs[UV_OFFSET(o, 0)], d->uvs[UV_OFFSET(o, 1)]);
			linear_math::Vector2 uv1(d->uvs[UV_OFFSET(o, 2)], d->uvs[UV_OFFSET(o, 3)]);
			linear_math::Vector2 uv2(d->uvs[UV_OFFSET(o, 4)], d->uvs[UV_OFFSET(o, 5)]);

			linear_math::Vector3 E1 = e1 - e0;
			linear_math::Vector3 E2 = e2 - e0;
			linear_math::Vector2 deltaUV1 = uv1 - uv0;
			linear_math::Vector2 deltaUV2 = uv2 - uv0;

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

			linear_math::Vector3 tangentVector = linear_math::normalize(linear_math::Vector3(tangents[0], tangents[1], tangents[2]));
			linear_math::Vector3 bitangentVector = linear_math::normalize(linear_math::Vector3(bitangents[0], bitangents[1], bitangents[2]));

			GMint verticesCount = component->getPrimitiveVerticesCountPtr()[i];
			for (GMint j = 0; j < verticesCount; j++)
			{
				d->tangents.push_back(tangentVector[0]);
				d->tangents.push_back(tangentVector[1]);
				d->tangents.push_back(tangentVector[2]);
				d->tangents.push_back(1.0f);

				d->bitangents.push_back(bitangentVector[0]);
				d->bitangents.push_back(bitangentVector[1]);
				d->bitangents.push_back(bitangentVector[2]);
				d->bitangents.push_back(1.0f);
			}
		}
	}
}
