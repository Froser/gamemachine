#include "stdafx.h"
#include "gmdemogameworld.h"
#include "foundation/gamemachine.h"
#include "gameobjects/gmcontrolgameobject.h"
#include "gameobjects/gm2dgameobject.h"

GMDemoGameWorld::GMDemoGameWorld()
{
}

void GMDemoGameWorld::renderScene()
{
	D(d);
	Base::renderScene();

	IGraphicEngine* engine = GameMachine::instance().getGraphicEngine();
	auto objs = d->gameObjects;
	if (GM.getCursor())
		objs.push_back(GM.getCursor());
	engine->drawObjects(objs.data(), objs.size());

	engine->beginBlend();
	auto& controls = getControlsGameObject();
	engine->drawObjects(controls.data(), controls.size());
	engine->endBlend();
}

bool GMDemoGameWorld::addObject(const GMString& name, GMGameObject* obj)
{
	D(d);
	auto r = d->renderList.find(name);
	if (r != d->renderList.end())
		return false;
	d->renderList[name] = obj;
	d->renderListInv[obj] = name;
	d->gameObjects.push_back(obj);
	addObjectAndInit(obj);
	return true;
}

bool GMDemoGameWorld::removeObject(const GMString& name)
{
	D(d);
	GMGameObject* object = findGameObject(name);
	if (object)
	{
		d->renderList.erase(name);
		d->renderListInv.erase(object);
		std::remove(d->gameObjects.begin(), d->gameObjects.end(), object);
		return GMGameWorld::removeObject(object);
	}
	return false;
}

GMGameObject* GMDemoGameWorld::findGameObject(const GMString& name)
{
	D(d);
	auto target = d->renderList.find(name);
	if (target == d->renderList.end())
		return nullptr;
	return target->second;
}

bool GMDemoGameWorld::findGameObject(const GMGameObject* obj, REF GMString& name)
{
	D(d);
	auto target = d->renderListInv.find(obj);
	if (target == d->renderListInv.end())
		return false;
	name = target->second;
	return true;
}

bool GMDemoGameWorld::removeObject(GMGameObject* obj)
{
	GMString name;
	bool found = findGameObject(obj, name);
	if (found)
	{
		return removeObject(name);
	}
	return false;
}
