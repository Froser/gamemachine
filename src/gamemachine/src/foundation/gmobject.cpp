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

void GMObject::connect(GMObject& sender, GMSignal signal, const GMEventCallback& callback)
{
	sender.addConnection(std::move(signal), *this, callback);
}

void GMObject::disconnect(GMObject& sender, GMSignal signal)
{
	sender.removeSignalAndConnection(std::move(signal), *this);
}

void GMObject::addConnection(GMSignal signal, GMObject& receiver, GMEventCallback callback)
{
	D(d);
	GMCallbackTarget target = { &receiver, callback };
	d->slots[signal].push_back(target);
	receiver.addConnection(this, std::move(signal));
}

void GMObject::removeSignalAndConnection(GMSignal signal, GMObject& receiver)
{
	D(d);
	// 移除自己的接收者
	removeSignal(signal, receiver);

	// 移除自己连接到的信号
	receiver.removeConnection(this, std::move(signal));
}

void GMObject::emit(GMSignal signal)
{
	D(d);
	if (d->slots.empty())
		return;

	auto& targets = d->slots[std::move(signal)];
	for (auto& target : targets)
	{
		target.callback(this, target.receiver);
	}
}

void GMObject::removeSignal(GMSignal signal, GMObject& receiver)
{
	D(d);
	auto& targets = d->slots[std::move(signal)];
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
	c.name = std::move(signal);
	d->connectionTargets.push_back(std::move(c));
}

void GMObject::removeConnection(GMObject* host, GMSignal signal)
{
	D(d);
	removeIf(d->connectionTargets, [&](auto iter) {
		return iter->name == signal && iter->host == host;
	});
}

// GMBuffer
GMBuffer::~GMBuffer()
{
	if (needRelease)
	{
		GM_delete_array(buffer);
	}
}

GMBuffer::GMBuffer(const GMBuffer& rhs)
{
	*this = rhs;
}

GMBuffer::GMBuffer(GMBuffer&& rhs) GM_NOEXCEPT
{
	swap(rhs);
}

GMBuffer& GMBuffer::operator =(GMBuffer&& rhs) GM_NOEXCEPT
{
	swap(rhs);
	return *this;
}

GMBuffer& GMBuffer::operator =(const GMBuffer& rhs)
{
	this->needRelease = rhs.needRelease;
	this->size = rhs.size;
	buffer = new GMbyte[this->size];
	memcpy_s(buffer, size, rhs.buffer, this->size);
	return *this;
}

void GMBuffer::convertToStringBuffer()
{
	GMbyte* newBuffer = new GMbyte[size + 1];
	memcpy_s(newBuffer, size, buffer, size);
	newBuffer[size] = 0;
	size++;
	if (needRelease && buffer)
		GM_delete_array(buffer);
	needRelease = true;
	buffer = newBuffer;
}

void GMBuffer::convertToStringBufferW()
{
	GMwchar* newBuffer = new GMwchar[size + 1];
	memcpy_s(newBuffer, size, buffer, size);
	newBuffer[size] = 0;
	size += sizeof(GMwchar);
	if (needRelease && buffer)
		GM_delete_array(buffer);
	needRelease = true;
	buffer = reinterpret_cast<GMbyte*>(newBuffer);
}

void GMBuffer::swap(GMBuffer& rhs)
{
	GM_SWAP(buffer, rhs.buffer);
	GM_SWAP(size, rhs.size);
	GM_SWAP(needRelease, rhs.needRelease);
}
