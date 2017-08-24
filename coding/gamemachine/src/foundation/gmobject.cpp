#include "stdafx.h"
#include "gmobject.h"
#include <utility>

template <typename ContainerType>
static size_t removeIf(ContainerType& container, std::function<bool(typename ContainerType::iterator)> pred)
{
	size_t cnt = 0;
	auto iter = container.begin();
	for (; iter != container.end(); iter++)
	{
		if (pred(iter))
		{
			iter = container.erase(iter);
			cnt++;
			if (iter == container.end())
				break;
		}
	}
	return cnt;
}

GMObject::GMObject(GMObject&& obj) noexcept
{
	*this = std::move(obj);
}

GMObject::~GMObject()
{
	releaseEvents();
}

GMObject& GMObject::operator=(GMObject&& obj) noexcept
{
	if (this != &obj)
	{
		swap(*this, obj);
		obj.dataWrapper()->m_data = nullptr;
	}
	return *this;
}

void GMObject::swap(GMObject& another)
{
	swap(*this, another);
}

void GMObject::swap(GMObject& a, GMObject& b)
{
	GMObjectPrivateWrapper<GMObject>* wrapperA = a.dataWrapper(),
		*wrapperB = b.dataWrapper();
	if (!wrapperA || !wrapperB)
		return;
	wrapperA->swap(wrapperB);
}

void GMObject::attachEvent(GMObject& sender, const char* eventName, const GMEventCallback& callback)
{
	sender.addEvent(eventName, *this, callback);
}

void GMObject::detachEvent(GMObject& sender, const char* eventName)
{
	sender.removeEventAndConnection(eventName, *this);
}

void GMObject::addEvent(const char* eventName, GMObject& receiver, const GMEventCallback& callback)
{
	D(d);
	GMCallbackTarget target = { &receiver, callback };
	d->events[eventName].push_back(target);
	receiver.addConnection(this, eventName);
}

void GMObject::removeEventAndConnection(const char* eventName, GMObject& receiver)
{
	D(d);
	removeEvent(eventName, receiver);
	receiver.removeConnection(this, eventName);
}

void GMObject::emitEvent(const char* eventName)
{
	D(d);
	auto& targets = d->events[eventName];
	for (auto& target : targets)
	{
		target.callback(*target.receiver);
	}
}

void GMObject::removeEvent(const char* eventName, GMObject& receiver)
{
	D(d);
	auto& targets = d->events[eventName];
	removeIf(targets, [&](auto iter) {
		return iter->receiver == &receiver;
	});
}

void GMObject::releaseEvents()
{
	// 释放此对象连接的所有事件
	D(d);
	for (auto& conns : d->connectionTargets)
	{
		conns.host->removeEvent(conns.name, *this);
	}
}

void GMObject::addConnection(GMObject* host, const char* eventName)
{
	D(d);
	GMConnectionTarget c;
	c.host = host;
	strcpy_s(c.name, eventName);
	d->connectionTargets.push_back(c);
}

void GMObject::removeConnection(GMObject* host, const char* eventName)
{
	D(d);
	removeIf(d->connectionTargets, [&](auto iter) {
		return strEqual(iter->name, eventName) && iter->host == host;
	});
}
