#include "stdafx.h"
#include "gmasync.h"

void* GMAsyncResult::state()
{
	return &buffer;
}

bool GMAsyncResult::isComplete()
{
	return complete;
}

void GMAsyncResult::wait()
{
	if (future.valid())
		future.wait();
}

void GMAsyncResult::setFuture(GMFuture<void> future)
{
	this->future = std::move(future);
}