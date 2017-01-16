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
#include "utilities/assert.h"
#include "gmengine/controller/animation.h"
#include "gmengine/elements/hallucinationgameobject.h"
#include "gmengine/elements/singleprimitivegameobject.h"
#include "gmengine/elements/gerstnerwavegameobject.h"

#define CREATE_FUNC static
#define RESOURCE_FUNC static

typedef void (*__ObjectCreateFunc)(IFactory* factory,
	ResourceContainer* resContainer,
	GMMap* map,
	const GMMapInstance* instance,
	const GMMapEntity* entity,
	const GMMapObject* object,
	OUT GameObject** gameObj);

static void createTexture(IFactory* factory, std::string& path, OUT ITexture** texture)
{
	Image* image;
	ImageReader::load(path.c_str(), &image);
	factory->createTexture(image, texture);
}

static btTransform getTransform(const GMMapInstance* instance)
{
	btTransform trans;
	trans.setIdentity();
	trans.setOrigin(btVector3(instance->position[0], instance->position[1], instance->position[2]));
	trans.setRotation(btQuaternion(btVector3(instance->rotation[0], instance->rotation[1], instance->rotation[2]), RAD(instance->rotation[3])));
	return trans;
}

static void setPropertiesFromInstance(GMMap* map, const GMMapInstance* instance, GameObject* gameObj)
{
	gameObj->setMass(instance->mass);

	LOG_ASSERT_MSG(instance->scale[0] == 0 || instance->scale[1] == 0 || instance->scale[2], "Invalid scale.");
	gameObj->setLocalScaling(btVector3(instance->scale[0], instance->scale[1], instance->scale[2]));
	gameObj->setTransform(getTransform(instance));
	gameObj->setFrictions(instance->frictions);

	for (GMuint i = 0; i < GMMapInstance::MAX_ANIMATION_TYPE; i++)
	{
		const GMMapKeyframes* keyframes = GMMap_find(map->animations, instance->animationRef[i]);
		if (keyframes)
		{
			Keyframes* coreKeyframes = nullptr;
			switch (keyframes->type)
			{
			case GMMapKeyframes::Rotation:
				coreKeyframes = &gameObj->getKeyframesRotation();
				break;
			case GMMapKeyframes::Translation:
				coreKeyframes = &gameObj->getKeyframesTranslation();
				break;
			case GMMapKeyframes::Scaling:
				coreKeyframes = &gameObj->getKeyframesScaling();
				break;
			default:
				ASSERT(false);
				LOG_ASSERT_MSG(false, "Wrong keyframes type.");
				break;
			}

			coreKeyframes->setFunctor(keyframes->functor);
			for (auto iter = keyframes->keyframes.begin(); iter != keyframes->keyframes.end(); iter++)
			{
				coreKeyframes->insert((*iter).keyframe);
			}
			gameObj->startAnimation(instance->animationDuration);
		}
	}
}

static void copyUniqueMaterialProperties(const Material& material, Object* coreObject)
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
static TextureFindResult getTextureForResourceContainer(ResourceContainer* resContainer, ID textureID)
{
	const TextureContainer::TextureItem* texture = resContainer->getTextureContainer().find(textureID);
	ITexture* coreTexture = texture ? texture->texture : nullptr;
	TextureType type = texture ? texture->type : TextureTypeResetStart;
	return std::make_pair(coreTexture, type);
}

static void loadObject(const char* path, IFactory* factory, OUT Object** obj)
{
	// 目前先用objreader来读取obj
	ObjReader reader(factory);
	reader.load(path, obj);
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
	if (object->path.length() > 0)
	{
		Object* coreObject = nullptr;
		std::string modelPath = getModelPath(map, object->path);
		loadObject(modelPath.c_str(), factory, &coreObject);
		*gameObj = new CubeGameObject(extents, coreObject);
	}
	else
	{
		*gameObj = new CubeGameObject(extents, object->magnification, materials);
	}

	if (object->collisionExtents[0] > 0 && object->collisionExtents[1] > 0 && object->collisionExtents[2] > 0)
		static_cast<CubeGameObject*>(*gameObj)->setCollisionExtents(
			btVector3(object->collisionExtents[0], object->collisionExtents[1], object->collisionExtents[2])
		);

	setPropertiesFromInstance(map, instance, *gameObj);

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

	if (object->path.length() > 0)
	{
		Object* coreObject = nullptr;
		std::string modelPath = getModelPath(map, object->path);
		loadObject(modelPath.c_str(), factory, &coreObject);
		*gameObj = new SphereGameObject(object->radius, coreObject);
	}
	else
	{
		*gameObj = new SphereGameObject(object->radius, object->slices, object->stacks, m);
	}
	setPropertiesFromInstance(map, instance, *gameObj);
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
	setPropertiesFromInstance(map, instance, *gameObj);
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

	Object* coreObject = nullptr;
	std::string modelPath = getModelPath(map, object->path);
	loadObject(modelPath.c_str(), factory, &coreObject);

	const GMMapMaterial* material = GMMap_find(map->materials, entity->materialRef[0]);
	if (material)
		copyUniqueMaterialProperties(material->material, coreObject);

	*gameObj = new ConvexHullGameObject(coreObject);
	setPropertiesFromInstance(map, instance, *gameObj);
}

CREATE_FUNC void createHallucination(IFactory* factory,
	ResourceContainer* resContainer,
	GMMap* map,
	const GMMapInstance* instance,
	const GMMapEntity* entity,
	const GMMapObject* object,
	OUT GameObject** gameObj)
{
	ASSERT(gameObj);

	Object* coreObject = nullptr;
	std::string modelPath = getModelPath(map, object->path);
	loadObject(modelPath.c_str(), factory, &coreObject);

	const GMMapMaterial* material = GMMap_find(map->materials, entity->materialRef[0]);
	if (material)
		copyUniqueMaterialProperties(material->material, coreObject);

	*gameObj = new HallucinationGameObject(coreObject);
	setPropertiesFromInstance(map, instance, *gameObj);
}

CREATE_FUNC void createSinglePrimitive(IFactory* factory,
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

	SinglePrimitiveGameObject::Size size = {
		object->height,
		object->radius,
		btVector3(object->width / 2, object->height / 2, object->depth / 2),
	};

	SinglePrimitiveGameObject* obj = new SinglePrimitiveGameObject(SinglePrimitiveGameObject::fromGMMapObjectType(object->type), size, m);
	*gameObj = obj;
	setPropertiesFromInstance(map, instance, obj);
}

CREATE_FUNC void createGerstnerWave(IFactory* factory,
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

	GerstnerWaveGameObject* obj = new GerstnerWaveGameObject(m, object->magnification, GerstnerWaveGameObject::defaultProperties());
	*gameObj = obj;
	setPropertiesFromInstance(map, instance, obj);
	obj->init();
}

struct __ObjectCreateFuncs
{
	__ObjectCreateFuncs()
	{
		__map[GMMapObject::Cube] = createCube;
		__map[GMMapObject::Sphere] = createSphere;
		__map[GMMapObject::Sky] = createSky;
		__map[GMMapObject::ConvexHull] = createConvexHull;
		__map[GMMapObject::Hallucination] = createHallucination;
		__map[GMMapObject::Capsule] = __map[GMMapObject::Cylinder] = __map[GMMapObject::Cone] = createSinglePrimitive;
		__map[GMMapObject::GerstnerWave] = createGerstnerWave;
	}

	std::map<GMMapObject::GMMapObjectType, __ObjectCreateFunc> __map;
};

static __ObjectCreateFunc& getObjectCreateFunc(GMMapObject::GMMapObjectType type)
{
	static __ObjectCreateFuncs createFuncs;
	return createFuncs.__map[type];
}

static void loadTextures(IGraphicEngine* engine, IFactory* factory, GMMap* map)
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

static void createGameObjectFromInstance(IGraphicEngine* engine, IFactory* factory, GMMap* map, const GMMapInstance* instance, OUT GameObject** gameObject)
{
	const GMMapEntity* entity = GMMap_find(map->entities, instance->entityRef);
	LOG_ASSERT_MSG(entity, "You may bind wrong entityref.");
	const GMMapObject* object = GMMap_find(map->objects, entity->objRef);
	LOG_ASSERT_MSG(object, "You may bind wrong objref.");
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
	else
	{
		return;
	}

	IFactory* factory = gm->getFactory();
	IGraphicEngine* engine = gm->getGraphicEngine();
	world->setGameMachine(gm);
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
			coreCharacter->setEyeOffset(character.eyeOffset);
			world->appendObject(coreCharacter);
			world->setMajorCharacter(coreCharacter);
		}
	}

	// 装载实例
	for (auto iter = map->instances.begin(); iter != map->instances.end(); iter++)
	{
		GameObject* gameObject;
		createGameObjectFromInstance(engine, factory, map, &(*iter), &gameObject);
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

	LOG_ASSERT_MSG(map->lights.size() > 0, "There is no light in the world.");
	world->initialize();
}