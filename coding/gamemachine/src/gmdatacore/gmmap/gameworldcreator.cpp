#include "stdafx.h"
#include <map>
#include "gameworldcreator.h"
#include "gmmap.h"
#include "gmengine/elements/gameworld.h"
#include "gmengine/elements/gameobject.h"
#include "gmdatacore/imagereader/imagereader.h"
#include "gmengine/controller/factory.h"
#include "gmengine/elements/cubegameobject.h"
#include "gmengine/elements/spheregameobject.h"

typedef void (*__ObjectCreateFunc)(IFactory* factory, GMMap* map, const GMMapInstance* instance, const GMMapEntity* entity, const GMMapObject* object, OUT GameObject** gameObj);

void createTexture(IFactory* factory, const char* path, OUT ITexture** texture)
{
	Image* image;
	ImageReader::load(path, &image);
	factory->createTexture(image, texture);
}

void createCube(IFactory* factory, GMMap* map, const GMMapInstance* instance, const GMMapEntity* entity, const GMMapObject* object, OUT GameObject** gameObj)
{
	ASSERT(gameObj);
	Material* materials = new Material[6];

	ASSERT(GMMapEntity::MAX_REF >= 6);
	for (GMuint i = 0; i < GMMapEntity::MAX_REF; i++)
	{
		// 读取纹理，纹理存放在textures目录下
		ITexture* coreTexture = nullptr;
		GMMapTexture textureKey = { entity->textureRef[i] };
		const GMMapTexture* texture = GMMap_find(map->textures, textureKey);
		if (texture)
		{
			std::string texImgPath = map->workingDir;
			texImgPath.append("textures/").append(texture->path);
			createTexture(factory, texImgPath.c_str(), &coreTexture);
		}

		// 拷贝材质
		GMMapMaterial materialKey = { entity->materialRef[i] };
		const GMMapMaterial* material = GMMap_find(map->materials, materialKey);
		if (material)
		{
			materials[i] = material->material;
			materials[i].textures->texture = coreTexture;
			materials[i].textures->type = texture->type;
		}
		else
		{
			materials[i] = Material();
		}
	}

	btTransform pos;
	pos.setIdentity();
	pos.setOrigin(btVector3(instance->x, instance->y, instance->z));
	btVector3 extents(object->width, object->height, object->depth);
	*gameObj = new CubeGameObject(extents, pos, materials);
	(*gameObj)->setMass(instance->mass);
	(*gameObj)->setLocalScaling(btVector3(instance->scale[0], instance->scale[1], instance->scale[2]));
	if (materials)
		delete[] materials;
}

void createSphere(IFactory* factory, GMMap* map, const GMMapInstance* instance, const GMMapEntity* entity, const GMMapObject* object, OUT GameObject** gameObj)
{
	ASSERT(gameObj);

	ITexture* coreTexture = nullptr;

	GMMapMaterial materialKey = { entity->materialRef[0] };
	const GMMapMaterial* material = GMMap_find(map->materials, materialKey);
	Material m = material ? material->material : Material();

	GMMapTexture textureKey = { entity->textureRef[0] };
	const GMMapTexture* texture = GMMap_find(map->textures, textureKey);
	if (texture)
	{
		std::string texImgPath = map->workingDir;
		texImgPath.append("textures/").append(texture->path);
		createTexture(factory, texImgPath.c_str(), &coreTexture);
		m.textures->type = texture->type;
	}
	m.textures->texture = coreTexture;

	btTransform pos;
	pos.setIdentity();
	pos.setOrigin(btVector3(instance->x, instance->y, instance->z));
	*gameObj = new SphereGameObject(object->radius, object->slices, object->stacks, pos, m);
	(*gameObj)->setMass(instance->mass);
	(*gameObj)->setLocalScaling(btVector3(instance->scale[0], instance->scale[1], instance->scale[2]));
}

struct __ObjectCreateFuncs
{
	__ObjectCreateFuncs()
	{
		__map[GMMapObject::Cube] = createCube;
		__map[GMMapObject::Sphere] = createSphere;
	}

	std::map<GMMapObject::GMMapObjectType, __ObjectCreateFunc> __map;
};

__ObjectCreateFunc& getObjectCreateFunc(GMMapObject::GMMapObjectType type)
{
	static __ObjectCreateFuncs createFuncs;
	return createFuncs.__map[type];
}

void createGameObjectFromInstance(IFactory* factory, GMMap* map, const GMMapInstance* instance, OUT GameObject** gameObject)
{
	GMMapEntity entityKey = { instance->entityRef };
	const GMMapEntity& entity = *map->entities.find(entityKey);

	GMMapObject objectKey = { entity.objRef };
	const GMMapObject& object = *map->objects.find(objectKey);

	getObjectCreateFunc(object.type)(factory, map, instance, &entity, &object, gameObject);
}

void GameWorldCreator::createGameWorld(IFactory* factory, GMMap* map, OUT GameWorld** gameWorld)
{
	GameWorld* world;
	if (gameWorld)
	{
		world = new GameWorld();
		*gameWorld = world;
	}

	IGraphicEngine* engine;
	factory->createGraphicEngine(&engine);
	world->setGraphicEngine(engine);

	for (auto iter = map->instances.begin(); iter != map->instances.end(); iter++)
	{
		GameObject* gameObject;
		createGameObjectFromInstance(factory, map, &(*iter), &gameObject);

		ObjectPainter* painter;
		factory->createPainter(engine, gameObject->getObject(), &painter);
		gameObject->getObject()->setPainter(painter);
		world->appendObject(gameObject);
	}

	world->initialize();
}