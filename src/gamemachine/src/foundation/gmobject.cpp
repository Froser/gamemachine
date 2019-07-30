#include "stdafx.h"
#include "gmobject.h"
#include "interfaces.h"
#include <utility>
#include "gmthread.h"

BEGIN_NS

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

GMObjectPrivate& GMObjectPrivate::dataOf(GMObject& obj)
{
	D_OF(d, &obj);
	return *d;
}

void GMObjectPrivate::addConnection(GMSignal sig, GMObject& receiver, GMEventCallback callback)
{
	GMCallbackTarget target = { &receiver, callback };
	objSlots[sig].push_back(target);
	P_D(pd);
	dataOf(receiver).addConnection(pd, std::move(sig));
}

void GMObjectPrivate::removeSignalAndConnection(GMSignal sig, GMObject& receiver)
{
	// 移除自己的接收者
	removeSignal(sig, receiver);

	// 移除自己连接到的信号
	P_D(pd);
	dataOf(receiver).removeConnection(pd, std::move(sig));
}

void GMObjectPrivate::removeSignal(GMSignal sig, GMObject& receiver)
{
	auto& targets = objSlots[std::move(sig)];
	removeIf(targets, [&](auto iter) {
		return iter->receiver == &receiver;
	});
}

void GMObjectPrivate::releaseConnections()
{
	P_D(pd);
	// 释放连接到此对象的信号
	if (!objSlots.empty())
	{
		for (auto& event : objSlots)
		{
			GMSignal name = event.first;
			for (auto& target : event.second)
			{
				dataOf(*target.receiver).removeConnection(pd, name);
			}
		}
		GMClearSTLContainer(objSlots);
	}

	// 释放此对象连接的所有信号
	if (!connectionTargets.empty())
	{
		for (auto& conns : connectionTargets)
		{
			dataOf(*conns.host).removeSignal(conns.name, *pd);
		}
	}
}

void GMObjectPrivate::addConnection(GMObject* host, GMSignal sig)
{
	GMConnectionTarget c;
	c.host = host;
	c.name = std::move(sig);
	connectionTargets.push_back(std::move(c));
}

void GMObjectPrivate::removeConnection(GMObject* host, GMSignal sig)
{
	removeIf(connectionTargets, [&](auto iter) {
		return iter->name == sig && iter->host == host;
	});
}

GMObject::GMObject()
{
	GM_CREATE_DATA();
	GM_SET_PD();

	D(d);
	d->tid = GMThread::getCurrentThreadId();
}

GMObject::~GMObject()
{
	if (GM_HAS_DATA())
	{
		D(d);
		d->releaseConnections();
	}
}

const GMMeta* GMObject::meta() const
{
	D(d);
	if (!d->metaRegistered)
	{
		d->metaRegistered = true;
		if (!const_cast<GMObject*>(this)->registerMeta())
			return nullptr;
	}
	return &d->meta;
}

void GMObject::connect(GMObject& sender, GMSignal sig, const GMEventCallback& callback)
{
	D_OF(d, &sender);
	d->addConnection(std::move(sig), *this, callback);
}

void GMObject::disconnect(GMObject& sender, GMSignal sig)
{
	D_OF(d, &sender);
	d->removeSignalAndConnection(std::move(sig), *this);
}

void GMObject::emitSignal(GMSignal sig)
{
	D(d);
	if (d->objSlots.empty())
		return;

	auto& targets = d->objSlots[std::move(sig)];
	for (auto& target : targets)
	{
		target.callback(this, target.receiver);
	}
}

GMThreadId GMObject::getThreadId() GM_NOEXCEPT
{
	D(d);
	return d->tid;
}

void GMObject::moveToThread(GMThreadId tid) GM_NOEXCEPT
{
	D(d);
	d->tid = tid;
}

END_NS