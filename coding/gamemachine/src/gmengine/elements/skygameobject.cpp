#include "stdafx.h"
#include "skygameobject.h"
#include "gameworld.h"
#include "character.h"
#include "utilities/vmath.h"

SkyGameObject::SkyGameObject(GMfloat len, ITexture* skyTexture)
	: m_length(len)
	, m_texture(skyTexture)
	, HallucinationGameObject(nullptr)
{
	initCoreObject();
	// 注意，Texture一定要是个CUBE_MAP
	GameObject::setMass(0.0f);
	setLocalScaling(btVector3(1, 1, 1));
}

void SkyGameObject::appendThisObjectToWorld(btDynamicsWorld* world)
{
}

void SkyGameObject::setWorld(GameWorld* world)
{
	world->setSky(this);
	GameObject::setWorld(world);
}

void SkyGameObject::getReadyForRender(DrawingList& list)
{
	// 需要把天空盒放到人物原点
	GameWorld* world = getWorld();
	Character* character = world->getMajorCharacter();
	CameraLookAt lookAt;
	Camera::calcCameraLookAt(character->getPositionState(), &lookAt);
	vmath::mat4 trans = vmath::translate(lookAt.position_x, lookAt.position_y, lookAt.position_z);
	DrawingItem item;
	memcpy(item.trans, trans, sizeof(vmath::mat4));
	item.gameObject = this;
	list.push_back(item);
}

void SkyGameObject::initCoreObject()
{
	GMfloat vertices[] = {
		//Front
		-m_length, m_length, m_length, 1,
		-m_length, -m_length, m_length, 1,
		m_length, -m_length, m_length, 1,
		m_length, m_length, m_length, 1,

		//Back
		-m_length, m_length, -m_length, 1,
		-m_length, -m_length, -m_length, 1,
		m_length, -m_length, -m_length, 1,
		m_length, m_length, -m_length, 1,

		//Left
		-m_length, m_length, -m_length, 1,
		-m_length, m_length, m_length, 1,
		-m_length, -m_length, m_length, 1,
		-m_length, -m_length, -m_length, 1,

		//Right
		m_length, m_length, -m_length, 1,
		m_length, m_length, m_length, 1,
		m_length, -m_length, m_length, 1,
		m_length, -m_length, -m_length, 1,

		//Up
		-m_length, m_length, -m_length, 1,
		-m_length, m_length, m_length, 1,
		m_length, m_length, m_length, 1,
		m_length, m_length, -m_length, 1,

		//Down
		-m_length, -m_length, -m_length, 1,
		-m_length, -m_length, m_length, 1,
		m_length, -m_length, m_length, 1,
		m_length, -m_length, -m_length, 1,
	};

	GMfloat normals[] = {
		0, 0, 1, 1,
		0, 0, 1, 1,
		0, 0, 1, 1,
		0, 0, 1, 1,

		0, 0, -1, 1,
		0, 0, -1, 1,
		0, 0, -1, 1,
		0, 0, -1, 1,

		-1, 0, 0, 1,
		-1, 0, 0, 1,
		-1, 0, 0, 1,
		-1, 0, 0, 1,

		1, 0, 0, 1,
		1, 0, 0, 1,
		1, 0, 0, 1,
		1, 0, 0, 1,

		0, 0, 1, 1,
		0, 0, 1, 1,
		0, 0, 1, 1,
		0, 0, 1, 1,

		0, 0, -1, 1,
		0, 0, -1, 1,
		0, 0, -1, 1,
		0, 0, -1, 1,
	};

	GMfloat uvs[] = {
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

	Object* coreObj = getObject();
	TextureInfo textureInfo = {
		m_texture, TextureTypeCubeMap
	};
	for (GMuint i = 0; i < 4 * 6 * 4; i++)
	{
		if (i % 16 == 0)
		{
			Component* c = new Component();
			c->setEdgeCountPerPolygon(4);
			c->setOffset(i / 4);
			c->getMaterial().textures[(int)(i / 16)] = textureInfo;
			c->getMaterial().Ka[0] = 1.0f;
			c->getMaterial().Ka[1] = 1.0f;
			c->getMaterial().Ka[2] = 1.0f;
			coreObj->appendComponent(c, 4);
		}
		coreObj->vertices().push_back(vertices[i]);
		coreObj->normals().push_back(normals[i]);

		if (i < 48)
			coreObj->uvs().push_back(uvs[i]);
	}
}