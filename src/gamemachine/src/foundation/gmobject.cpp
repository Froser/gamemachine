#include "stdafx.h"
#include "gmobject.h"
#include "interfaces.h"
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

GMObject::~GMObject()
{
	releaseEvents();
}

GMObject::GMObject(GMObject&& e) noexcept
{
	gmSwap(*this, e);
}

GMObject& GMObject::operator=(GMObject&& e) noexcept
{
	gmSwap(*this, e);
	return *this;
}

void GMObject::attachEvent(GMObject& sender, GMEventName eventName, const GMEventCallback& callback)
{
	sender.addEvent(eventName, *this, callback);
}

void GMObject::detachEvent(GMObject& sender, GMEventName eventName)
{
	sender.removeEventAndConnection(eventName, *this);
}

void GMObject::addEvent(GMEventName eventName, GMObject& receiver, const GMEventCallback& callback)
{
	D(d);
	GMCallbackTarget target = { &receiver, callback };
	d->events[eventName].push_back(target);
	receiver.addConnection(this, eventName);
}

void GMObject::removeEventAndConnection(GMEventName eventName, GMObject& receiver)
{
	D(d);
	removeEvent(eventName, receiver);
	receiver.removeConnection(this, eventName);
}

void GMObject::emitEvent(GMEventName eventName)
{
	D(d);
	if (d->events.empty())
		return;

	auto& targets = d->events[eventName];
	for (auto& target : targets)
	{
		target.callback(this, target.receiver);
	}
}

void GMObject::removeEvent(GMEventName eventName, GMObject& receiver)
{
	D(d);
	auto& targets = d->events[eventName];
	removeIf(targets, [&](auto iter) {
		return iter->receiver == &receiver;
	});
}

void GMObject::releaseEvents()
{
	D(d);
	// 释放连接到此对象的事件
	if (!d->events.empty())
	{
		for (auto& event : d->events)
		{
			GMEventName name = event.first;
			for (auto& target : event.second)
			{
				target.receiver->removeConnection(this, name);
			}
		}
	}

	// 释放此对象连接的所有事件
	if (!d->connectionTargets.empty())
	{
		for (auto& conns : d->connectionTargets)
		{
			conns.host->removeEvent(conns.name, *this);
		}
	}

	GMClearSTLContainer(d->events);
}

void GMObject::addConnection(GMObject* host, GMEventName eventName)
{
	D(d);
	GMConnectionTarget c;
	c.host = host;
	c.name = eventName;
	d->connectionTargets.push_back(c);
}

void GMObject::removeConnection(GMObject* host, GMEventName eventName)
{
	D(d);
	removeIf(d->connectionTargets, [&](auto iter) {
		return iter->name == eventName && iter->host == host;
	});
}
