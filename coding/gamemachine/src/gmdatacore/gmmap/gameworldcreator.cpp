#include "stdafx.h"
#include <map>
#include <set>
#include "gameworldcreator.h"
#include "gmmap.h"
#include "gmengine/elements/gameworld.h"
#include "gmengine/elements/gameobject.h"
#include "gmdatacore/imagereader/imagereader.h"
#include "gmengine/elements/cubegameobject.h"
#include "gmengine/elements/spheregameobject.h"
#include "gmengine/elements/skygameobject.h"
#include "gmengine/elements/convexhullgameobject.h"
#include "gmengine/elements/character.h"
#include "gmdatacore/objreader/objreader.h"
#include "gmengine/controller/resource_container.h"
#include "gmengine/elements/gamelight.h"
#include "gmengine/controller/gamemachine.h"

#define CREATE_FUNC
#define RESOURCE_FUNC

typedef void (*__ObjectCreateFunc)(IFactory* factory,
	ResourceContainer* resContainer,
	GMMap* map,
	const GMMapInstance* instance,
	const GMMapEntity* entity,
	const GMMapObject* object,
	OUT GameObject** gameObj);

void createTexture(IFactory* factory, std::string& path, OUT ITexture** texture)
{
	Image* image;
	ImageReader::load(path.c_str(), &image);
	factory->createTexture(image, texture);
}

btTransform getTransform(const GMMapInstance* instance)
{
	btTransform trans;
	trans.setIdentity();
	trans.setOrigin(btVector3(instance->position[0], instance->position[1], instance->position[2]));
	trans.setRotation(btQuaternion(btVector3(instance->rotation[0], instance->rotation[1], instance->rotation[2]), RAD(instance->rotation[3])));
	return trans;
}

void setPropertiesFromInstance(const GMMapInstance* instance, GameObject* gameObj)
{
	gameObj->setMass(instance->mass);
	gameObj->setLocalScaling(btVector3(instance->scale[0], instance->scale[1], instance->scale[2]));
	gameObj->setTransform(getTransform(instance));
}

void copyUniqueMaterialProperties(const Material& material, Object* coreObject)
{
	// 拷贝obj文件中不存在的一些属性
	for (auto iter = coreObject->getComponents().begin(); iter != coreObject->getComponents().end(); iter++)
	{
		Component* c = *iter;
		c->getMaterial().Ke[0] = material.Ke[0];
		c->getMaterial().Ke[1] = material.Ke[1];
		c->getMaterial().Ke[2] = material.Ke[2];
	}
}

RESOURCE_FUNC std::string getTexturePath(GMMap* map, const GMMapString& texturePath)
{
	std::string dir = map->workingDir;
	return dir.append("textures/").append(texturePath);
}

RESOURCE_FUNC std::string getModelPath(GMMap* map, const GMMapString& name)
{
	std::string dir = map->workingDir;
	return dir.append("models/").append(name).append("/entity");
}

typedef std::pair<ITexture*, TextureType> TextureFindResult;
TextureFindResult getTextureForResourceContainer(ResourceContainer* resContainer, ID textureID)
{
	const TextureContainer::TextureItem* texture = resContainer->getTextureContainer().find(textureID);
	ITexture* coreTexture = texture ? texture->texture : nullptr;
	TextureType type = texture ? texture->type : TextureTypeResetStart;
	return std::make_pair(coreTexture, type);
}

CREATE_FUNC void createCube(IFactory* factory,
	ResourceContainer* resContainer,
	GMMap* map,
	const GMMapInstance* instance,
	const GMMapEntity* entity,
	const GMMapObject* object,
	OUT GameObject** gameObj)
{
	ASSERT(gameObj);
	Material* materials = new Material[6];

	ASSERT(GMMapEntity::MAX_REF >= 6);
	for (GMuint i = 0; i < GMMapEntity::MAX_REF; i++)
	{
		TextureFindResult texture = getTextureForResourceContainer(resContainer, entity->textureRef[i]);

		// 拷贝材质
		const GMMapMaterial* material = GMMap_find(map->materials, entity->materialRef[i]);
		if (material)
		{
			materials[i] = material->material;
			materials[i].textures->texture = texture.first;
			materials[i].textures->type = texture.second;
		}
		else
		{
			materials[i] = Material();
		}
	}

	btVector3 extents(object->width, object->height, object->depth);
	*gameObj = new CubeGameObject(extents, object->magnification, materials);
	setPropertiesFromInstance(instance, *gameObj);
	if (materials)
		delete[] materials;
}

CREATE_FUNC void createSphere(IFactory* factory,
	ResourceContainer* resContainer,
	GMMap* map,
	const GMMapInstance* instance,
	const GMMapEntity* entity,
	const GMMapObject* object,
	OUT GameObject** gameObj)
{
	ASSERT(gameObj);


	const GMMapMaterial* material = GMMap_find(map->materials, entity->materialRef[0]);
	Material m = material ? material->material : Material();

	TextureFindResult texture = getTextureForResourceContainer(resContainer, entity->textureRef[0]);
	m.textures->texture = texture.first;
	m.textures->type = texture.second;

	*gameObj = new SphereGameObject(object->radius, object->slices, object->stacks, m);
	setPropertiesFromInstance(instance, *gameObj);
}

CREATE_FUNC void createSky(IFactory* factory,
	ResourceContainer* resContainer,
	GMMap* map,
	const GMMapInstance* instance,
	const GMMapEntity* entity,
	const GMMapObject* object,
	OUT GameObject** gameObj)
{
	ASSERT(gameObj);

	TextureFindResult texture = getTextureForResourceContainer(resContainer, entity->textureRef[0]);
	ASSERT(texture.first != nullptr);

	*gameObj = new SkyGameObject(object->radius, texture.first);
	setPropertiesFromInstance(instance, *gameObj);
}

CREATE_FUNC void createConvexHull(IFactory* factory,
	ResourceContainer* resContainer,
	GMMap* map,
	const GMMapInstance* instance,
	const GMMapEntity* entity,
	const GMMapObject* object,
	OUT GameObject** gameObj)
{
	ASSERT(gameObj);

	// 目前先用objreader来读取obj
	ObjReader reader;
	Object* coreObject;
	std::string modelPath = getModelPath(map, object->path);
	reader.load(modelPath.c_str(), &coreObject);

	const GMMapMaterial* material = GMMap_find(map->materials, entity->materialRef[0]);
	if (material)
		copyUniqueMaterialProperties(material->material, coreObject);

	*gameObj = new ConvexHullGameObject(coreObject);
	setPropertiesFromInstance(instance, *gameObj);
}

struct __ObjectCreateFuncs
{
	__ObjectCreateFuncs()
	{
		__map[GMMapObject::Cube] = createCube;
		__map[GMMapObject::Sphere] = createSphere;
		__map[GMMapObject::Sky] = createSky;
		__map[GMMapObject::ConvexHull] = createConvexHull;
	}

	std::map<GMMapObject::GMMapObjectType, __ObjectCreateFunc> __map;
};

__ObjectCreateFunc& getObjectCreateFunc(GMMapObject::GMMapObjectType type)
{
	static __ObjectCreateFuncs createFuncs;
	return createFuncs.__map[type];
}

void loadTextures(IGraphicEngine* engine, IFactory* factory, GMMap* map)
{
	ResourceContainer* resContainer = engine->getResourceContainer();
	TextureContainer& textures = resContainer->getTextureContainer();
	for (auto iter = map->textures.begin(); iter != map->textures.end(); iter++)
	{
		TextureContainer::TextureItem item;
		const GMMapTexture& texture = (*iter);
		item.id = texture.id;
		item.type = texture.type;
		createTexture(factory, getTexturePath(map, texture.path), &item.texture);
		resContainer->getTextureContainer().insert(item);
	}
}

void createGameObjectFromInstance(IGraphicEngine* engine, IFactory* factory, GMMap* map, const GMMapInstance* instance, OUT GameObject** gameObject)
{
	const GMMapEntity* entity = GMMap_find(map->entities, instance->entityRef);
	const GMMapObject* object = GMMap_find(map->objects, entity->objRef);
	ResourceContainer* resContainer = engine->getResourceContainer();
	getObjectCreateFunc(object->type)(factory, resContainer, map, instance, entity, object, gameObject);
}

void GameWorldCreator::createGameWorld(GameMachine* gm, GMMap* map, OUT GameWorld** gameWorld)
{
	GameWorld* world;
	if (gameWorld)
	{
		world = new GameWorld();
		*gameWorld = world;
	}

	IFactory* factory = gm->getFactory();
	IGraphicEngine* engine = gm->getGraphicEngine();
	world->setGraphicEngine(engine);
	loadTextures(engine, factory, map);

	// 装载设定
	GMMapSettings& settings = map->settings;
	{
		// 万物当先，先要初始化重力
		GMMapSettings::Gravity& gravity = settings.gravity;
		world->setGravity(gravity.vector[0], gravity.vector[1], gravity.vector[2]);

		GMMapSettings::Character& character = settings.character;
		if (character.height > 0 && character.radius > 0)
		{
			btTransform trans;
			trans.setIdentity();
			trans.setOrigin(btVector3(character.position[0], character.position[1], character.position[2]));
			Character* coreCharacter = new Character(trans, character.radius, character.height, character.stepHeight);
			coreCharacter->setCanFreeMove(!!character.freemove);
			coreCharacter->setJumpSpeed(btVector3(character.jumpSpeed[0], character.jumpSpeed[1], character.jumpSpeed[2]));
			coreCharacter->setFallSpeed(character.fallSpeed);
			coreCharacter->setMoveSpeed(character.movespeed);
			world->appendObject(coreCharacter);
			world->setMajorCharacter(coreCharacter);
		}
	}

	// 装载实例
	for (auto iter = map->instances.begin(); iter != map->instances.end(); iter++)
	{
		GameObject* gameObject;
		createGameObjectFromInstance(engine, factory, map, &(*iter), &gameObject);

		ObjectPainter* painter;
		factory->createPainter(engine, gameObject->getObject(), &painter);
		gameObject->getObject()->setPainter(painter);
		world->appendObject(gameObject);
	}

	// 装载光源
	for (auto iter = map->lights.begin(); iter != map->lights.end(); iter++)
	{
		const GMMapLight& ioLight = (*iter);
		GameLight* gameLight = nullptr;
		factory->createLight(ioLight.type, &gameLight);
		
		if (gameLight)
		{
			gameLight->setColor(ioLight.rgb);
			gameLight->setPosition(ioLight.position);
			gameLight->setRange(ioLight.range);
			gameLight->setWorld(world);
			gameLight->setShadowSource(!!ioLight.shadow);
			world->appendLight(gameLight);
		}
	}

	world->initialize();
}