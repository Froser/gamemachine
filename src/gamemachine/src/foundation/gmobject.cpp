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
	releaseConnections();
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

void GMObject::connect(GMObject& sender, GMSignal signal, const GMEventCallback& callback)
{
	sender.addConnection(signal, *this, callback);
}

void GMObject::disconnect(GMObject& sender, GMSignal signal)
{
	sender.removeSignalAndConnection(signal, *this);
}

void GMObject::addConnection(GMSignal signal, GMObject& receiver, GMEventCallback callback)
{
	D(d);
	GMCallbackTarget target = { &receiver, callback };
	d->slots[signal].push_back(target);
	receiver.addConnection(this, signal);
}

void GMObject::removeSignalAndConnection(GMSignal signal, GMObject& receiver)
{
	D(d);
	// 移除自己的接收者
	removeSignal(signal, receiver);

	// 移除自己连接到的信号
	receiver.removeConnection(this, signal);
}

void GMObject::emit(GMSignal signal)
{
	D(d);
	if (d->slots.empty())
		return;

	auto& targets = d->slots[signal];
	for (auto& target : targets)
	{
		target.callback(this, target.receiver);
	}
}

void GMObject::removeSignal(GMSignal signal, GMObject& receiver)
{
	D(d);
	auto& targets = d->slots[signal];
	removeIf(targets, [&](auto iter) {
		return iter->receiver == &receiver;
	});
}

void GMObject::releaseConnections()
{
	D(d);
	// 释放连接到此对象的信号
	if (!d->slots.empty())
	{
		for (auto& event : d->slots)
		{
			GMSignal name = event.first;
			for (auto& target : event.second)
			{
				target.receiver->removeConnection(this, name);
			}
		}
	}

	// 释放此对象连接的所有信号
	if (!d->connectionTargets.empty())
	{
		for (auto& conns : d->connectionTargets)
		{
			conns.host->removeSignal(conns.name, *this);
		}
	}

	GMClearSTLContainer(d->slots);
}

void GMObject::addConnection(GMObject* host, GMSignal signal)
{
	D(d);
	GMConnectionTarget c;
	c.host = host;
	c.name = signal;
	d->connectionTargets.push_back(c);
}

void GMObject::removeConnection(GMObject* host, GMSignal signal)
{
	D(d);
	removeIf(d->connectionTargets, [&](auto iter) {
		return iter->name == signal && iter->host == host;
	});
}
