#include "stdafx.h"
#include "object.h"
#include "foundation/linearmath.h"
#include <algorithm>

#define VERTEX_DEMENSION 4 //顶点的维度，最高维度是齐次维度，恒为1

ObjectPainter::ObjectPainter(Object* obj)
	: m_object(obj)
{

}

Object* ObjectPainter::getObject()
{
	return m_object;
}

Object::~Object()
{
	if (m_painter)
		m_painter->dispose();

	BEGIN_FOREACH_OBJ(this, childObj)
	{
		if (childObj)
			delete childObj;
	}
	END_FOREACH_OBJ
}

Component::Component(ChildObject* parent)
	: m_offset(0)
	, m_primitiveCount(0)
	, m_parent(parent)
{
	memset(&m_shader, 0, sizeof(m_shader));

	setVertexOffset(m_parent->vertices().size() / VERTEX_DEMENSION);
}

Component::~Component()
{
	TextureInfo& ti = m_shader.texture;
	if (ti.autorelease)
	{
		TextureFrames* frames = m_shader.texture.textures;
		for (GMint i = 0; i < TEXTURE_INDEX_MAX; i++)
		{
			for (GMint j = 0; j < frames[i].frameCount; j++)
			{
				ITexture* t = frames[i].frames[i];
				if (t)
					delete t;
			}
		}
	}
}

// 设置此component的第一个顶点位于ChildObject.vertices()中的偏移位置
// 一般不需要手动调用
void Component::setVertexOffset(GMuint offset)
{
	m_offset = offset;
}

void Component::beginFace()
{
	m_currentFaceVerticesCount = 0;
}

void Component::vertex(GMfloat x, GMfloat y, GMfloat z)
{
	AlignedVector<Object::DataType>& vertices = m_parent->vertices();
	vertices.push_back(x);
	vertices.push_back(y);
	vertices.push_back(z);
	vertices.push_back(1.0f);
	m_currentFaceVerticesCount++;
}

void Component::normal(GMfloat x, GMfloat y, GMfloat z)
{
	AlignedVector<Object::DataType>& normals = m_parent->normals();
	normals.push_back(x);
	normals.push_back(y);
	normals.push_back(z);
	normals.push_back(1.0f);
}

void Component::uv(GMfloat u, GMfloat v)
{
	AlignedVector<Object::DataType>& uvs = m_parent->uvs();
	uvs.push_back(u);
	uvs.push_back(v);
}

void Component::lightmap(GMfloat u, GMfloat v)
{
	AlignedVector<Object::DataType>& lightmaps = m_parent->lightmaps();
	lightmaps.push_back(u);
	lightmaps.push_back(v);
}

void Component::endFace()
{
	m_vertexOffsets.push_back(m_primitiveVertices.empty() ?
		m_offset : m_vertexOffsets.back() + m_primitiveVertices.back()
	);
	m_primitiveVertices.push_back(m_currentFaceVerticesCount);
	m_primitiveCount++;
}

ChildObject::ChildObject()
	: m_arrayId(0)
	, m_bufferId(0)
	, m_type(NormalObject)
	, m_mode(Triangle_Fan)
	, m_name("default")
{
}

ChildObject::ChildObject(const std::string& name)
	: m_arrayId(0)
	, m_bufferId(0)
	, m_type(NormalObject)
	, m_mode(Triangle_Fan)
{
	m_name = name;
}

ChildObject::~ChildObject()
{
	for (auto iter = m_components.begin(); iter != m_components.end(); iter++)
	{
		delete *iter;
	}
}

void ChildObject::appendComponent(AUTORELEASE Component* component)
{
	ASSERT(m_components.find(component) == m_components.end());
	m_components.push_back(component);
}

void ChildObject::calculateTangentSpace()
{
	if (m_uvs.size() == 0)
		return;

	for (auto iter = m_components.begin(); iter != m_components.end(); iter++)
	{
		Component* component = (*iter);
		for (GMuint i = 0; i < component->getPrimitiveCount(); i++)
		{
			GMint offset = component->getOffsetPtr()[i] / VERTEX_DEMENSION;
			GMint edgeCount = component->getPrimitiveVerticesCountPtr()[i];

			// 开始计算每条边切线空间
			for (GMint j = 0; j < edgeCount; j++)
			{
				linear_math::Vector3 e0(m_vertices[(offset + j) * VERTEX_DEMENSION], m_vertices[(offset + j) * VERTEX_DEMENSION + 1], m_vertices[(offset + j) * VERTEX_DEMENSION + 3]);
				linear_math::Vector3 e1, e2;
				if (j == edgeCount - 2)
				{
					e1 = linear_math::Vector3(m_vertices[(offset + j + 1) * VERTEX_DEMENSION], m_vertices[(offset + j + 1) * VERTEX_DEMENSION + 1], m_vertices[(offset + j + 1) * VERTEX_DEMENSION + 2]);
					e2 = linear_math::Vector3(m_vertices[offset * VERTEX_DEMENSION], m_vertices[offset * VERTEX_DEMENSION + 1], m_vertices[offset * VERTEX_DEMENSION + 3]);
				}
				else if (j == edgeCount - 1)
				{
					e1 = linear_math::Vector3(m_vertices[offset * VERTEX_DEMENSION], m_vertices[offset * 2 + 1], m_vertices[offset * VERTEX_DEMENSION + 2]);
					e2 = linear_math::Vector3(m_vertices[(offset + 1) * VERTEX_DEMENSION], m_vertices[(offset + 1) * VERTEX_DEMENSION + 1], m_vertices[offset * VERTEX_DEMENSION + 2]);
				}
				else
				{
					e1 = linear_math::Vector3(m_vertices[(offset + j + 1) * VERTEX_DEMENSION], m_vertices[(offset + j + 1) * VERTEX_DEMENSION + 1], m_vertices[(offset + j + 1) * VERTEX_DEMENSION + 2]);
					e2 = linear_math::Vector3(m_vertices[(offset + j + 2) * VERTEX_DEMENSION], m_vertices[(offset + j + 2) * VERTEX_DEMENSION + 1], m_vertices[(offset + j + 1) * VERTEX_DEMENSION + 2]);
				}

				linear_math::Vector2 uv0(m_uvs[(offset + j) * 2], m_uvs[(offset + j) * 2 + 1]);
				linear_math::Vector2 uv1, uv2;
				if (j == edgeCount - 2)
				{
					uv1 = linear_math::Vector2(m_uvs[(offset + j + 1) * 2], m_uvs[(offset + j + 1) * 2 + 1]);
					uv2 = linear_math::Vector2(m_uvs[offset * 2], m_uvs[offset * 2 + 1]);
				}
				else if (j == edgeCount - 1)
				{
					uv1 = linear_math::Vector2(m_uvs[offset * 2], m_uvs[offset * 2 + 1]);
					uv1 = linear_math::Vector2(m_uvs[(offset + 1) * 2], m_uvs[(offset + 1) * 2 + 1]);
				}
				else
				{
					uv1 = linear_math::Vector2(m_uvs[(offset + j + 1) * 2], m_uvs[(offset + j + 1) * 2 + 1]);
					uv2 = linear_math::Vector2(m_uvs[(offset + j + 2) * 2], m_uvs[(offset + j + 2) * 2 + 1]);
				}

				linear_math::Vector3 E1 = e1 - e0;
				linear_math::Vector3 E2 = e2 - e0;
				linear_math::Vector2 deltaUV1 = uv1 - uv0;
				linear_math::Vector2 deltaUV2 = uv2 - uv0;

				GMfloat t1 = deltaUV1[0], b1 = deltaUV1[1], t2 = deltaUV2[0], b2 = deltaUV2[1];
				GMfloat s = 1.0f / (t1 * b2 - b1 * t2);
				{
					GMfloat t[3] = {
						s * (b2 * E1[0] - b1 * E2[0]),
						s * (b2 * E1[1] - b1 * E2[1]),
						s * (b2 * E1[2] - b1 * E2[2])
					};
					linear_math::Vector3 v_t = linear_math::normalize(linear_math::Vector3(t[0], t[1], t[2]));
					m_tangents.push_back(v_t[0]);
					m_tangents.push_back(v_t[1]);
					m_tangents.push_back(v_t[2]);
					m_tangents.push_back(1.0f);
				}
				{
					GMfloat t[3] = {
						s * (t1 * E2[0] - t2 * E1[0]),
						s * (t1 * E2[1] - t2 * E1[1]),
						s * (t1 * E2[2] - t2 * E1[2])
					};
					linear_math::Vector3 v_t = linear_math::normalize(linear_math::Vector3(t[0], t[1], t[2]));
					m_bitangents.push_back(v_t[0]);
					m_bitangents.push_back(v_t[1]);
					m_bitangents.push_back(v_t[2]);
					m_bitangents.push_back(1.0f);
				}
			}
		}
	}
}
