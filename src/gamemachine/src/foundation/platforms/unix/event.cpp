#include "stdafx.h"
#include "check.h"
#include "../../utilities/tools.h"
#include <pthread.h>

GM_PRIVATE_OBJECT_UNALIGNED(GMEvent)
{
	void* handle = nullptr;
};

struct GMEventHandleStruct
{
	pthread_mutex_t mutex;
	pthread_cond_t signal;
	bool manualReset;
	bool state;
};

GMEvent::GMEvent(bool manualReset, bool initialState)
{
	GM_CREATE_DATA(GMEvent);

	D(d);
	GMEventHandleStruct* handle = new GMEventHandleStruct();
	d->handle = (void*) handle;
	pthread_mutex_init(&handle->mutex, NULL);
	pthread_cond_init(&handle->signal, NULL);
	handle->manualReset = manualReset;
	handle->state = initialState;
}

GMEvent::~GMEvent()
{
	D(d);
	GMEventHandleStruct* handle = (GMEventHandleStruct*)d->handle;
	pthread_mutex_destroy(&handle->mutex);
	pthread_cond_destroy(&handle->signal);
	GM_delete(handle);
	d->handle = nullptr;
}

void GMEvent::wait(GMuint32 milliseconds)
{
	D(d);
	GMEventHandleStruct* handle = (GMEventHandleStruct*)d->handle;
	pthread_mutex_lock(&handle->mutex);
	while (handle->state)
	{
		pthread_cond_wait(&handle->signal, &handle->mutex);
	}
	if (!handle->manualReset)
		handle->state = false;
	pthread_mutex_unlock(&handle->mutex);
}

void GMEvent::set()
{
	D(d);
	GMEventHandleStruct* handle = (GMEventHandleStruct*)d->handle;
	pthread_mutex_lock(&handle->mutex);
	handle->state = true;
	pthread_mutex_unlock(&handle->mutex);
	pthread_cond_signal(&handle->signal);
}

void GMEvent::reset()
{
	D(d);
	GMEventHandleStruct* handle = (GMEventHandleStruct*)d->handle;
	pthread_mutex_lock(&handle->mutex);
	handle->state = false;
	pthread_mutex_unlock(&handle->mutex);
}

GMAutoResetEvent::GMAutoResetEvent(bool initialState)
	: GMEvent(false, initialState)
{
}

GMManualResetEvent::GMManualResetEvent(bool initialState)
	: GMEvent(true, initialState)
{
}