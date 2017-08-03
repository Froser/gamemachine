#include "stdafx.h"
#include "gmdemogameworld.h"
#include "foundation/gamemachine.h"

GMDemoGameWorld::GMDemoGameWorld()
{
}

void GMDemoGameWorld::renderGameWorld()
{
	D(d);
	Base::renderGameWorld();

	IGraphicEngine* engine = GameMachine::instance().getGraphicEngine();
	for (auto& object : d->renderList)
	{
		engine->drawObject(object.second);
	}
}

GMPhysicsWorld* GMDemoGameWorld::physicsWorld()
{
	return nullptr;
}

bool GMDemoGameWorld::appendObject(const GMString& name, GMGameObject* obj)
{
	D(d);
	auto& r = d->renderList.find(name);
	if (r != d->renderList.end())
		return false;
	d->renderList[name] = obj;
	d->renderListInv[obj] = name;
	appendObjectAndInit(obj);
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