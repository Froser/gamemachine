#include "stdafx.h"
#include "gmthreads.h"
#if _WINDOWS
#	include <process.h>
#endif
#include <mutex>
#include <thread>

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
}

void GMThread::start()
{
	D(d);

#if _WINDOWS
	d->event.reset();
#endif
	d->handle = GMThreadHandle(&GMThread::threadCallback, this);
	if (d->callback)
		d->callback->onCreateThread(this);
}

void GMThread::wait(GMuint milliseconds)
{
	D(d);
#if _WINDOWS
	HANDLE* handle = (HANDLE*)d->handle.native_handle();
	::WaitForSingleObject(handle, milliseconds == 0 ? INFINITE : milliseconds);
#endif
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
#if _WINDOWS
	return std::this_thread::get_id();
#endif
}

void GMThread::sleep(GMint miliseconds)
{
#if _WINDOWS
	::Sleep(miliseconds);
#else
	GM_ASSERT(false);
#endif
}