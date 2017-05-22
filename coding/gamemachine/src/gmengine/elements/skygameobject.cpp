#include "stdafx.h"
#include "skygameobject.h"

static linear_math::Vector2 uvs[24] = {
	linear_math::Vector2(0, 0),
	linear_math::Vector2(0, 1),
	linear_math::Vector2(1, 1),
	linear_math::Vector2(1, 0),

	linear_math::Vector2(0, 0),
	linear_math::Vector2(0, 1),
	linear_math::Vector2(1, 1),
	linear_math::Vector2(1, 0),

	linear_math::Vector2(0, 0),
	linear_math::Vector2(0, 1),
	linear_math::Vector2(1, 1),
	linear_math::Vector2(1, 0),

	linear_math::Vector2(0, 0),
	linear_math::Vector2(0, 1),
	linear_math::Vector2(1, 1),
	linear_math::Vector2(1, 0),

	linear_math::Vector2(0, 0),
	linear_math::Vector2(0, 1),
	linear_math::Vector2(1, 1),
	linear_math::Vector2(1, 0),

	/*
	linear_math::Vector2(0, 0),
	linear_math::Vector2(0, 1),
	linear_math::Vector2(1, 1),
	linear_math::Vector2(1, 0),
	*/
};

SkyGameObject::SkyGameObject(const Shader& shader, const linear_math::Vector3& min, const linear_math::Vector3& max)
	: GameObject(nullptr)
	, m_shader(shader)
	, m_min(min)
	, m_max(max)
{
	Object* obj = nullptr;
	createSkyBox(&obj);
	setObject(obj);
}

void SkyGameObject::createSkyBox(OUT Object** obj)
{
	linear_math::Vector3 vertices[20] = {
		//Front
		linear_math::Vector3(m_min[0], m_max[1], m_max[2]),
		linear_math::Vector3(m_min[0], m_min[1], m_max[2]),
		linear_math::Vector3(m_max[0], m_min[1], m_max[2]),
		linear_math::Vector3(m_max[0], m_max[1], m_max[2]),

		//Back
		linear_math::Vector3(m_min[0], m_max[1], m_min[2]),
		linear_math::Vector3(m_min[0], m_min[1], m_min[2]),
		linear_math::Vector3(m_max[0], m_min[1], m_min[2]),
		linear_math::Vector3(m_max[0], m_max[1], m_min[2]),

		//Left
		linear_math::Vector3(m_min[0], m_max[1], m_min[2]),
		linear_math::Vector3(m_min[0], m_max[1], m_max[2]),
		linear_math::Vector3(m_min[0], m_min[1], m_max[2]),
		linear_math::Vector3(m_min[0], m_min[1], m_min[2]),

		//Right
		linear_math::Vector3(m_max[0], m_max[1], m_min[2]),
		linear_math::Vector3(m_max[0], m_max[1], m_max[2]),
		linear_math::Vector3(m_max[0], m_min[1], m_max[2]),
		linear_math::Vector3(m_max[0], m_min[1], m_min[2]),

		//Up
		linear_math::Vector3(m_min[0], m_max[1], m_min[2]),
		linear_math::Vector3(m_min[0], m_max[1], m_max[2]),
		linear_math::Vector3(m_max[0], m_max[1], m_max[2]),
		linear_math::Vector3(m_max[0], m_max[1], m_min[2]),

		//Down
		/*
		linear_math::Vector3(m_min[0], m_min[1], m_min[2]),
		linear_math::Vector3(m_min[0], m_min[1], m_max[2]),
		linear_math::Vector3(m_max[0], m_min[1], m_max[2]),
		linear_math::Vector3(m_max[0], m_min[1], m_min[2]),
		*/
	};

	// Scaling surface
	const GMint SCALING = 2;
	linear_math::Vector3 center = (m_min + m_max) / 2;
	linear_math::Matrix4x4 transScale = linear_math::scale(linear_math::Vector3(SCALING, 1, SCALING));
	for (GMuint i = 0; i < 20; i++)
	{
		linear_math::Matrix4x4 transRestore = linear_math::translate(center);
		linear_math::Matrix4x4 transMoveToAxisOrigin = linear_math::translate(-center);
		linear_math::Matrix4x4 transFinal = transRestore * transScale * transMoveToAxisOrigin;
		
		// 因为linear_math::mat为列优先，所以与vec相乘的时候应该先transpose
		linear_math::Vector4 pt = linear_math::Vector4(vertices[i], 1) * transFinal.transpose();
		vertices[i] = linear_math::Vector3(pt[0], pt[1], pt[2]);
	}

	Object* object = new Object();
	*obj = object;

	ChildObject* child = new ChildObject();
	child->setType(ChildObject::Sky);

	Component* component = new Component(child);
	component->getShader() = m_shader;

	// We don't draw surface beneath us
	for (GMuint i = 0; i < 5; i++)
	{
		component->beginFace();
		component->vertex(vertices[i * 4][0], vertices[i * 4][1], vertices[i * 4][2]);
		component->vertex(vertices[i * 4 + 1][0], vertices[i * 4 + 1][1], vertices[i * 4 + 1][2]);
		component->vertex(vertices[i * 4 + 2][0], vertices[i * 4 + 2][1], vertices[i * 4 + 2][2]);
		component->vertex(vertices[i * 4 + 3][0], vertices[i * 4 + 3][1], vertices[i * 4 + 3][2]);
		component->uv(uvs[i * 4][0], uvs[i * 4][1]);
		component->uv(uvs[i * 4 + 1][0], uvs[i * 4 + 1][1]);
		component->uv(uvs[i * 4 + 2][0], uvs[i * 4 + 2][1]);
		component->uv(uvs[i * 4 + 3][0], uvs[i * 4 + 3][1]);
		component->endFace();
	}
	child->appendComponent(component);
	object->append(child);
}