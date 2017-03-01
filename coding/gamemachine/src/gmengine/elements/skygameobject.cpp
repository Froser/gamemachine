#include "stdafx.h"
#include "skygameobject.h"

static vmath::vec2 uvs[24] = {
	vmath::vec2(0, 0),
	vmath::vec2(0, 1),
	vmath::vec2(1, 1),
	vmath::vec2(1, 0),

	vmath::vec2(0, 0),
	vmath::vec2(0, 1),
	vmath::vec2(1, 1),
	vmath::vec2(1, 0),

	vmath::vec2(0, 0),
	vmath::vec2(0, 1),
	vmath::vec2(1, 1),
	vmath::vec2(1, 0),

	vmath::vec2(0, 0),
	vmath::vec2(0, 1),
	vmath::vec2(1, 1),
	vmath::vec2(1, 0),

	vmath::vec2(0, 0),
	vmath::vec2(0, 1),
	vmath::vec2(1, 1),
	vmath::vec2(1, 0),

	/*
	vmath::vec2(0, 0),
	vmath::vec2(0, 1),
	vmath::vec2(1, 1),
	vmath::vec2(1, 0),
	*/
};

SkyGameObject::SkyGameObject(const Shader& shader, vmath::vec3& min, vmath::vec3 max)
	: HallucinationGameObject(nullptr)
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
	vmath::vec3 vertices[20] = {
		//Front
		vmath::vec3(m_min[0], m_max[1], m_max[2]),
		vmath::vec3(m_min[0], m_min[1], m_max[2]),
		vmath::vec3(m_max[0], m_min[1], m_max[2]),
		vmath::vec3(m_max[0], m_max[1], m_max[2]),

		//Back
		vmath::vec3(m_min[0], m_max[1], m_min[2]),
		vmath::vec3(m_min[0], m_min[1], m_min[2]),
		vmath::vec3(m_max[0], m_min[1], m_min[2]),
		vmath::vec3(m_max[0], m_max[1], m_min[2]),

		//Left
		vmath::vec3(m_min[0], m_max[1], m_min[2]),
		vmath::vec3(m_min[0], m_max[1], m_max[2]),
		vmath::vec3(m_min[0], m_min[1], m_max[2]),
		vmath::vec3(m_min[0], m_min[1], m_min[2]),

		//Right
		vmath::vec3(m_max[0], m_max[1], m_min[2]),
		vmath::vec3(m_max[0], m_max[1], m_max[2]),
		vmath::vec3(m_max[0], m_min[1], m_max[2]),
		vmath::vec3(m_max[0], m_min[1], m_min[2]),

		//Up
		vmath::vec3(m_min[0], m_max[1], m_min[2]),
		vmath::vec3(m_min[0], m_max[1], m_max[2]),
		vmath::vec3(m_max[0], m_max[1], m_max[2]),
		vmath::vec3(m_max[0], m_max[1], m_min[2]),

		//Down
		/*
		vmath::vec3(m_min[0], m_min[1], m_min[2]),
		vmath::vec3(m_min[0], m_min[1], m_max[2]),
		vmath::vec3(m_max[0], m_min[1], m_max[2]),
		vmath::vec3(m_max[0], m_min[1], m_min[2]),
		*/
	};

	// Scaling surface
	const GMint SCALING = 2;
	vmath::vec3 center = (m_min + m_max) / 2;
	vmath::mat4 transScale = vmath::scale(vmath::vec3(SCALING, 1, SCALING));
	for (GMuint i = 0; i < 20; i++)
	{
		vmath::mat4 transRestore = vmath::translate(center);
		vmath::mat4 transMoveToAxisOrigin = vmath::translate(-center);
		vmath::mat4 transFinal = transRestore * transScale * transMoveToAxisOrigin;
		
		// 因为vmath::mat为列优先，所以与vec相乘的时候应该先transpose
		vmath::vec4 pt = vmath::vec4(vertices[i], 1) * transFinal.transpose();
		vertices[i] = vmath::vec3(pt[0], pt[1], pt[2]);
	}

	Object* object = new Object();
	*obj = object;

	Material material = { 0 };
	material.Ka[0] = 1.0f; material.Ka[1] = 1.0f; material.Ka[2] = 1.0f;
	material.shader = m_shader;

	ChildObject* child = new ChildObject();
	child->setType(ChildObject::Sky);

	Component* component = new Component(child);
	component->getMaterial() = material;

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