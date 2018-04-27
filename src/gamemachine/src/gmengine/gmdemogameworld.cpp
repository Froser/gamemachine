#include "stdafx.h"
#include "gmdemogameworld.h"
#include "foundation/gamemachine.h"
#include "gameobjects/gmcontrolgameobject.h"
#include "gameobjects/gm2dgameobject.h"

GMDemoGameWorld::GMDemoGameWorld()
{
}

GMDemoGameWorld::~GMDemoGameWorld()
{
	D(d);
	GMClearSTLContainer(d->renderList);
}

void GMDemoGameWorld::renderScene()
{
	D(d);
	Base::renderScene();

	IGraphicEngine* engine = GM.getGraphicEngine();
	if (getRenderPreference() == GMRenderPreference::PreferForwardRendering)
	{
		engine->draw(d->forwardGameObjects.data(), d->forwardGameObjects.size(), nullptr, 0);
		engine->draw(d->deferredGameObjects.data(), d->deferredGameObjects.size(), nullptr, 0);
	}
	else
	{
		engine->draw(d->forwardGameObjects.data(), d->forwardGameObjects.size(), 
			d->deferredGameObjects.data(), d->deferredGameObjects.size());
	}

	engine->beginBlend();
	auto& controls = getControlsGameObject();
	engine->draw(controls.data(), controls.size(), nullptr, 0);
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
	if (obj->canDeferredRendering())
		d->deferredGameObjects.push_back(obj);
	else
		d->forwardGameObjects.push_back(obj);
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
		if (object->canDeferredRendering())
			std::remove(d->deferredGameObjects.begin(), d->deferredGameObjects.end(), object);
		else
			std::remove(d->forwardGameObjects.begin(), d->forwardGameObjects.end(), object);
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
