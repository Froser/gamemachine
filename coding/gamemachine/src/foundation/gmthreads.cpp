#include "stdafx.h"
#include "gmthreads.h"
#if _WINDOWS
#	include <process.h>
#endif
#include <mutex>
#include <thread>

#if GM_USE_PTHREAD
void* threadCallback(void* thread)
{
	GMThread* t = gmobject_cast<GMThread*>(thread);
	GMThread::Data* d = t->data();

	d->state = Running;
	if (d->callback)
		d->callback->beforeRun(this);
	run();
	if (d->callback)
		d->callback->afterRun(this);
	d->state = Finished;
	d->event.set();
	d->done = true;
}
#endif // GM_USE_PTHREAD

GMThread::GMThread()
{
	D(d);
	d->state = NotRunning;
	d->callback = nullptr;
}

void GMThread::threadCallback()
{
	D(d);
	d->state = Running;
	if (d->callback)
		d->callback->beforeRun(this);
	run();
	if (d->callback)
		d->callback->afterRun(this);
	d->state = Finished;
	d->event.set();
	d->done = true;
}

void GMThread::start()
{
	D(d);
#if GM_USE_PTHREAD
	pthread_create(&d->handle->thread, nullptr, threadCallback, this);
#else
	d->handle = GMThreadHandle(&GMThread::threadCallback, this);
#endif // GM_USE_PTHREAD

	d->event.reset();
	d->done = false;
	if (d->callback)
		d->callback->onCreateThread(this);
}

void GMThread::wait(GMuint milliseconds)
{
	D(d);
	d->event.wait();
}

void GMThread::setCallback(IThreadCallback* callback)
{
	D(d);
	d->callback = callback;
}

void GMThread::terminate()
{
	D(d);
#if _WINDOWS
	HANDLE* handle = (HANDLE*)d->handle.native_handle();
	::TerminateThread(handle, 0);
#endif
}

GMThreadHandle::id GMThread::getCurrentThreadId()
{
#if GM_USE_PTHREAD
	return pthread_self();
#else
	return std::this_thread::get_id();
#endif
}

GMThread::GMThread(GMThread&& t) noexcept
{
	swap(t);
}

void GMThread::sleep(GMint miliseconds)
{
#if _WINDOWS
	::Sleep(miliseconds);
#else
	GM_ASSERT(false);
#endif
}
