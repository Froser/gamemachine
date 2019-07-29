#include "stdafx.h"
#include "check.h"
#include <gmthread.h>
#include <signal.h>
#include <unistd.h>

BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMThread)
{
	IThreadCallback* callback = nullptr;
	GMThreadHandle handle = 0;
	ThreadState state;
	ThreadPriority priority = ThreadPriority::Normal;
	GMThreadAttr attr;
	bool done = false;
};

namespace
{
	void* threadProc(void* param)
	{
		GMThread* thread = static_cast<GMThread*>(param);
		GM_PRIVATE_NAME(GMThread)* d = thread->_thread_private();
		d->state = ThreadState::Running;
		if (d->callback)
			d->callback->beforeRun(thread);
		thread->run();
		if (d->callback)
			d->callback->afterRun(thread);
		d->state = ThreadState::Finished;
		d->done = true;
		return NULL;
	}
}

GMThread::GMThread()
{
	GM_CREATE_DATA();

	D(d);
	d->state = ThreadState::NotRunning;
	d->callback = nullptr;

	pthread_attr_init(&d->attr);
}

GMThread::~GMThread()
{
	D(d);
	pthread_attr_destroy(&d->attr);
}

void GMThread::setPriority(ThreadPriority p)
{
	D(d);
	//TODO Useless
	d->priority = p;
}

void GMThread::start()
{
	D(d);
	if (d->callback)
		d->callback->onCreateThread(this);
	d->done = false;
	if (pthread_create(&d->handle, &d->attr, threadProc, this))
	{
		gm_error(gm_dbg_wrap("Create thread error."));
		GM_ASSERT(false);
	}
}

void GMThread::setCallback(IThreadCallback* callback)
{
	D(d);
	d->callback = callback;
}

bool GMThread::join(GMuint32 milliseconds)
{
	D(d);
	pthread_join(d->handle, NULL);
}

GMThreadId GMThread::getThreadId()
{
	D(d);
	return d->handle;
}

GMThreadId GMThread::getCurrentThreadId()
{
	return pthread_self();
}

void GMThread::terminateThread(GMint32 ret)
{
	D(d);
	pthread_kill(d->handle, 0);
}

void GMThread::sleep(GMint32 milliseconds)
{
	usleep(milliseconds * 1000);
}

GMThreadHandle& GMThread::handle()
{
	D(d);
	return d->handle;
}

bool GMThread::isDone()
{
	D(d);
	return d->done;
}

// Mutex
GM_PRIVATE_OBJECT_UNALIGNED(GMMutex)
{
	pthread_mutex_t mutex;
};

GMMutex::GMMutex()
{
	GM_CREATE_DATA();
	D(d);
	pthread_mutex_init(&d->mutex, NULL);
}

GMMutex::~GMMutex()
{
	D(d);
	pthread_mutex_destroy(&d->mutex);
}

void GMMutex::lock()
{
	D(d);
	pthread_mutex_lock(&d->mutex);
}

void GMMutex::unlock()
{
	D(d);
	pthread_mutex_unlock(&d->mutex);
}

END_NS