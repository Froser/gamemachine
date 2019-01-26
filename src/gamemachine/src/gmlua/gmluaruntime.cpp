#include "stdafx.h"
#include "gmluaruntime.h"
#include <gmobject.h>
#include <gmthread.h>

GMLuaRuntime::~GMLuaRuntime()
{
	D(d);
	for (auto& obj : d->autoReleasePool)
	{
		(const_cast<IDestroyObject*>(obj))->destroy();
	}
	GMClearSTLContainer(d->autoReleasePool);
}

bool GMLuaRuntime::addObject(IDestroyObject* object)
{
	D(d);
	bool added = false;
	GMMutex m;
	m.lock();
	if (d->autoReleasePool.find(object) == d->autoReleasePool.end())
	{
		d->autoReleasePool.insert(object);; // 放入托管列表
		added = true;
	}
	m.unlock();
	return added;
}

bool GMLuaRuntime::detachObject(IDestroyObject* object)
{
	D(d);
	GMMutex m;
	m.lock();
	GMsize_t cnt = d->autoReleasePool.erase(object);
	m.unlock();
	return cnt > 0;
}

bool GMLuaRuntime::containsObject(IDestroyObject* object)
{
	D(d);
	GMMutex m;
	m.lock();
	bool contains = (d->autoReleasePool.find(object) != d->autoReleasePool.end());
	m.unlock();
	return contains;
}