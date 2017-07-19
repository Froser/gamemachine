#include "stdafx.h"
#include "object.h"
#include "foundation/linearmath.h"
#include <algorithm>

#define VERTEX_DEMENSION 4 //顶点的维度，最高维度是齐次维度，恒为1

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
	setVertexOffset(d->parentMesh->vertices().size() / VERTEX_DEMENSION);
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
	AlignedVector<Object::DataType>& vertices = d->parentMesh->vertices();
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
		linear_math::Vector3 e0(d->vertices[0], d->vertices[1], d->vertices[2]);
		linear_math::Vector3 e1(d->vertices[4], d->vertices[5], d->vertices[6]);
		linear_math::Vector3 e2(d->vertices[8], d->vertices[9], d->vertices[10]);

		linear_math::Vector2 uv0(d->uvs[0], d->uvs[1]);
		linear_math::Vector2 uv1(d->uvs[2], d->uvs[3]);
		linear_math::Vector2 uv2(d->uvs[4], d->uvs[5]);

		linear_math::Vector3 E1 = e1 - e0;
		linear_math::Vector3 E2 = e2 - e0;
		linear_math::Vector2 deltaUV1 = uv1 - uv0;
		linear_math::Vector2 deltaUV2 = uv2 - uv0;

		const GMfloat &t1 = deltaUV1[0], &b1 = deltaUV1[1], &t2 = deltaUV2[0], &b2 = deltaUV2[1];
		GMfloat s = 1.0f / (t1 * b2 - b1 * t2);

		GMfloat tangents[3] = {
			s * (b2 * E1[0] - b1 * E2[0]),
			s * (b2 * E1[1] - b1 * E2[1]),
			s * (b2 * E1[2] - b1 * E2[2])
		};
		GMfloat bitangents[3] = {
			s * (t1 * E2[0] - t2 * E1[0]),
			s * (t1 * E2[1] - t2 * E1[1]),
			s * (t1 * E2[2] - t2 * E1[2])
		};

		linear_math::Vector3 tangentVector = linear_math::normalize(linear_math::Vector3(tangents[0], tangents[1], tangents[2]));
		linear_math::Vector3 bitangentVector = linear_math::normalize(linear_math::Vector3(bitangents[0], bitangents[1], bitangents[2]));
		for (GMuint i = 0; i < component->getPrimitiveCount(); i++)
		{
			// 每一个多边形拥有一个切线空间，这意味着多于3个点的多边形需要共面，否则切线空间会有问题
			GMint offset = component->getOffsetPtr()[i] / VERTEX_DEMENSION;
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
		break;
	}
}
