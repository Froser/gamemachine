#include "stdafx.h"
#include <gmthread.h>
#if GM_WINDOWS
#	include <process.h>
#endif

namespace
{
	unsigned WINAPI threadProc(PVOID pvParam)
	{
		GMThread* thread = static_cast<GMThread*>(pvParam);
		GM_PRIVATE_NAME(GMThread)* d = thread->_thread_private();
		d->state = Running;
		if (d->callback)
			d->callback->beforeRun(thread);
		thread->run();
		if (d->callback)
			d->callback->afterRun(thread);
		d->state = Finished;
		d->done = true;
		return 0;
	}
}

GMThread::GMThread()
{
	D(d);
	d->state = NotRunning;
	d->callback = nullptr;
}

void GMThread::start()
{
	D(d);
	if (d->callback)
		d->callback->onCreateThread(this);
	d->done = false;
	d->handle = (GMlong)_beginthreadex(nullptr, 0, &threadProc, this, 0, nullptr);
}

void GMThread::setCallback(IThreadCallback* callback)
{
	D(d);
	d->callback = callback;
}

void GMThread::terminateThread(GMint ret)
{
	D(d);
	::TerminateThread((HANDLE)d->handle, ret);
}

bool GMThread::join(GMuint milliseconds)
{
	D(d);
	return ::WaitForSingleObject((HANDLE)d->handle, !milliseconds ? INFINITE : milliseconds) == WAIT_OBJECT_0;
}

GMThreadId GMThread::getThreadId()
{
	D(d);
	return ::GetThreadId((HANDLE)d->handle);
}

GMThreadId GMThread::getCurrentThreadId()
{
	return ::GetCurrentThreadId();
}

void GMThread::sleep(GMint milliseconds)
{
	::Sleep(milliseconds);
}

GMMutex::GMMutex()
{
	D(d);
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, FALSE };
	d->mutex = ::CreateMutex(&sa, FALSE, L"");
}

GMMutex::~GMMutex()
{
	D(d);
	::CloseHandle(d->mutex);
}

void GMMutex::lock()
{
	D(d);
	::WaitForSingleObject(d->mutex, INFINITE);
}

void GMMutex::unlock()
{
	D(d);
	::ReleaseMutex(d->mutex);
}