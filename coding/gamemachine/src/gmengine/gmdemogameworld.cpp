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
	appendObjectAndInit(obj);
	return true;
}

GMGameObject* GMDemoGameWorld::getGameObject(const GMString& name)
{
	D(d);
	auto target = d->renderList.find(name);
	if (target == d->renderList.end())
		return nullptr;
	return target->second;
}