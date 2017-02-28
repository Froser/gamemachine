#include "stdafx.h"
#include "skygameobject.h"

static GMfloat uvs[24][2] = {
	0, 0,
	0, 1,
	1, 1,
	1, 0,

	0, 0,
	0, 1,
	1, 1,
	1, 0,

	0, 0,
	0, 1,
	1, 1,
	1, 0,

	0, 0,
	0, 1,
	1, 1,
	1, 0,

	0, 0,
	0, 1,
	1, 1,
	1, 0,

	0, 0,
	0, 1,
	1, 1,
	1, 0,
};

SkyGameObject::SkyGameObject(ITexture* texture, vmath::vec3& min, vmath::vec3 max)
	: HallucinationGameObject(nullptr)
	, m_texture(texture)
	, m_min(min)
	, m_max(max)
{
	Object* obj = nullptr;
	createSkyBox(&obj);
	setObject(obj);
}

void SkyGameObject::createSkyBox(OUT Object** obj)
{
	GMfloat vertices[24][3] = {
		//Front
		m_min[0], m_max[1], m_max[2],
		m_min[0], m_min[1], m_max[2],
		m_max[0], m_min[1], m_max[2],
		m_max[0], m_max[1], m_max[2],

		//Back
		m_min[0], m_max[1], m_min[2],
		m_min[0], m_min[1], m_min[2],
		m_max[0], m_min[1], m_min[2],
		m_max[0], m_max[1], m_min[2],

		//Left
		m_min[0], m_max[1], m_min[2],
		m_min[0], m_max[1], m_max[2],
		m_min[0], m_min[1], m_max[2],
		m_min[0], m_min[1], m_min[2],

		//Right
		m_max[0], m_max[1], m_min[2],
		m_max[0], m_max[1], m_max[2],
		m_max[0], m_min[1], m_max[2],
		m_max[0], m_min[1], m_min[2],

		//Up
		m_min[0], m_max[1], m_min[2],
		m_min[0], m_max[1], m_max[2],
		m_max[0], m_max[1], m_max[2],
		m_max[0], m_max[1], m_min[2],

		//Down
		m_min[0], m_min[1], m_min[2],
		m_min[0], m_min[1], m_max[2],
		m_max[0], m_min[1], m_max[2],
		m_max[0], m_min[1], m_min[2],
	};

	Object* object = new Object();
	*obj = object;

	Material material = { 0 };
	material.Ka[0] = 1.0f; material.Ka[1] = 1.0f; material.Ka[2] = 1.0f;
	material.shader.texture.textureFrames[TEXTURE_INDEX_AMBIENT].frameCount = 1;
	material.shader.texture.textureFrames[TEXTURE_INDEX_AMBIENT].textures[0] = m_texture;
	material.shader.cull = GMS_NONE;
	material.shader.noDepthTest = true;

	ChildObject* child = new ChildObject();
	child->setType(ChildObject::Sky);
	Component* component = new Component(child);
	component->getMaterial() = material;

	for (GMuint i = 0; i < 6; i++)
	{
		component->beginFace();
		component->vertex(vertices[i * 4][0], vertices[i * 4][1], vertices[i * 4][2]);
		component->vertex(vertices[i * 4 + 1][0], vertices[i * 4 + 1][1], vertices[i * 4 + 1][2]);
		component->vertex(vertices[i * 4 + 2][0], vertices[i * 4 + 2][1], vertices[i * 4 + 2][2]);
		component->vertex(vertices[i * 4 + 3][0], vertices[i * 4 + 3][1], vertices[i * 4 + 3][2]);
		component->uv(uvs[i * 4][0], vertices[i * 4][1]);
		component->uv(uvs[i * 4 + 1][0], vertices[i * 4 + 1][1]);
		component->endFace();
	}
	child->appendComponent(component);
	object->append(child);
}