#include "stdafx.h"
#include "gmluaruntime.h"
#include <gmobject.h>
#include <gmthread.h>

GMLuaRuntime::~GMLuaRuntime()
{
	D(d);
	for (auto& obj : d->autoReleasePool)
	{
		GM_delete(const_cast<GMObject*>(obj));
	}
	for (auto& obj : d->autoReleasePool2)
	{
		(const_cast<IDestroyObject*>(obj))->destroy();
	}
	GMClearSTLContainer(d->autoReleasePool);
	GMClearSTLContainer(d->autoReleasePool2);
}

bool GMLuaRuntime::addObject(GMObject* object)
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

bool GMLuaRuntime::detachObject(GMObject* object)
{
	D(d);
	GMMutex m;
	m.lock();
	GMsize_t cnt = d->autoReleasePool.erase(object);
	m.unlock();
	return cnt > 0;
}

bool GMLuaRuntime::containsObject(GMObject* object)
{
	D(d);
	GMMutex m;
	m.lock();
	bool contains = (d->autoReleasePool.find(object) != d->autoReleasePool.end());
	m.unlock();
	return contains;
}

bool GMLuaRuntime::addObject(IDestroyObject* object)
{
	D(d);
	bool added = false;
	GMMutex m;
	m.lock();
	if (d->autoReleasePool2.find(object) == d->autoReleasePool2.end())
	{
		d->autoReleasePool2.insert(object);; // 放入托管列表
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
	GMsize_t cnt = d->autoReleasePool2.erase(object);
	m.unlock();
	return cnt > 0;
}

bool GMLuaRuntime::containsObject(IDestroyObject* object)
{
	D(d);
	GMMutex m;
	m.lock();
	bool contains = (d->autoReleasePool2.find(object) != d->autoReleasePool2.end());
	m.unlock();
	return contains;
}