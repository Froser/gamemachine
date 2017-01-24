#include "stdafx.h"
#include "object.h"
#include "utilities/vmath.h"

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

Component::Component()
	: m_offset(0)
	, m_verticesCount(0)
	, m_polygonCount(0)
{
	memset(&m_material, 0, sizeof(m_material));
}

Component::~Component()
{
	for (GMuint i = 0; i < MaxTextureCount; i++)
	{
		TextureInfo texture = getMaterial().textures[i];
		if (texture.autorelease)
		{
			if (texture.texture)
				delete texture.texture;
			if (texture.normalMapping)
				delete texture.normalMapping;
		}
	}
}

ChildObject::ChildObject()
	: m_arrayId(0)
	, m_bufferId(0)
	, m_type(NormalObject)
	, m_mode(Triangle_Fan)
	, m_name("default")
	, m_visibility(true)
{
}

ChildObject::ChildObject(const std::string& name)
	: m_arrayId(0)
	, m_bufferId(0)
	, m_type(NormalObject)
	, m_mode(Triangle_Fan)
	, m_visibility(true)
{
	m_name = name;
}

ChildObject::~ChildObject()
{
	for (auto iter = m_components.begin(); iter != m_components.cend(); iter++)
	{
		delete *iter;
	}
}

void ChildObject::appendComponent(AUTORELEASE Component* component, GMuint verticesCount)
{
	component->m_verticesCount = verticesCount;
	m_components.push_back(component);
}

void ChildObject::disposeMemory()
{
	m_vertices.clear();
	m_normals.clear();
	m_uvs.clear();
}

void ChildObject::calculateTangentSpace()
{
	if (m_uvs.size() == 0)
		return;

	for (auto iter = m_components.begin(); iter != m_components.end(); iter++)
	{
		Component* component = (*iter);
		for (GMuint i = 0; i < component->getPolygonCount(); i++)
		{
			GMint offset = component->getOffsetPtr()[i];
			GMint edgeCount = component->getEdgeCountPtr()[i];

			// 开始计算每条边切线空间
			for (GMuint j = 0; j < edgeCount; j++)
			{
				vmath::vec3 e0(m_vertices[(offset + j) * 4], m_vertices[(offset + j) * 4 + 1], m_vertices[(offset + j) * 4 + 3]);
				vmath::vec3 e1, e2;
				if (j == edgeCount - 2)
				{
					e1 = vmath::vec3(m_vertices[(offset + j + 1) * 4], m_vertices[(offset + j + 1) * 4 + 1], m_vertices[(offset + j + 1) * 4 + 2]);
					e2 = vmath::vec3(m_vertices[offset * 4], m_vertices[offset * 4 + 1], m_vertices[offset * 4 + 3]);
				}
				else if (j == edgeCount - 1)
				{
					e1 = vmath::vec3(m_vertices[offset * 4], m_vertices[offset * 2 + 1], m_vertices[offset * 4 + 2]);
					e2 = vmath::vec3(m_vertices[(offset + 1) * 4], m_vertices[(offset + 1) * 4 + 1], m_vertices[offset * 4 + 2]);
				}
				else
				{
					e1 = vmath::vec3(m_vertices[(offset + j + 1) * 4], m_vertices[(offset + j + 1) * 4 + 1], m_vertices[(offset + j + 1) * 4 + 2]);
					e2 = vmath::vec3(m_vertices[(offset + j + 2) * 4], m_vertices[(offset + j + 2) * 4 + 1], m_vertices[(offset + j + 1) * 4 + 2]);
				}

				vmath::vec2 uv0(m_uvs[(offset + j) * 2], m_uvs[(offset + j) * 2 + 1]);
				vmath::vec2 uv1, uv2;
				if (j == edgeCount - 2)
				{
					uv1 = vmath::vec2(m_uvs[(offset + j + 1) * 2], m_uvs[(offset + j + 1) * 2 + 1]);
					uv2 = vmath::vec2(m_uvs[offset * 2], m_uvs[offset * 2 + 1]);
				}
				else if (j == edgeCount - 1)
				{
					uv1 = vmath::vec2(m_uvs[offset * 2], m_uvs[offset * 2 + 1]);
					uv1 = vmath::vec2(m_uvs[(offset + 1) * 2], m_uvs[(offset + 1) * 2 + 1]);
				}
				else
				{
					uv1 = vmath::vec2(m_uvs[(offset + j + 1) * 2], m_uvs[(offset + j + 1) * 2 + 1]);
					uv2 = vmath::vec2(m_uvs[(offset + j + 2) * 2], m_uvs[(offset + j + 2) * 2 + 1]);
				}

				vmath::vec3 E1 = e1 - e0;
				vmath::vec3 E2 = e2 - e0;
				vmath::vec2 deltaUV1 = uv1 - uv0;
				vmath::vec2 deltaUV2 = uv2 - uv0;

				GMfloat t1 = deltaUV1[0], b1 = deltaUV1[1], t2 = deltaUV2[0], b2 = deltaUV2[1];
				GMfloat s = 1.0f / (t1 * b2 - b1 * t2);
				GMfloat t[3] = {
					s * (b2 * E1[0] - b1 * E2[0]),
					s * (b2 * E1[1] - b1 * E2[1]),
					s * (b2 * E1[2] - b1 * E2[2])
				};
				vmath::vec3 v_t = vmath::normalize(vmath::vec3(t[0], t[1], t[2]));
				m_tangents.push_back(v_t[0]);
				m_tangents.push_back(v_t[1]);
				m_tangents.push_back(v_t[2]);
				m_tangents.push_back(1.0f);
			}
		}
	}
}