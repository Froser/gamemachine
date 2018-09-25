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
		d->state = ThreadState::Running;
		if (d->callback)
			d->callback->beforeRun(thread);
		thread->run();
		if (d->callback)
			d->callback->afterRun(thread);
		d->state = ThreadState::Finished;
		d->done = true;
		return 0;
	}

	DWORD toWin32ThreadPriority(ThreadPriority t)
	{
		switch (t)
		{
		case ThreadPriority::TimeCritial:
			return THREAD_PRIORITY_TIME_CRITICAL;
		case ThreadPriority::Highest:
			return THREAD_PRIORITY_HIGHEST;
		case ThreadPriority::AboveNormal:
			return THREAD_PRIORITY_ABOVE_NORMAL;
		case ThreadPriority::Normal:
			return THREAD_PRIORITY_NORMAL;
		case ThreadPriority::BelowNormal:
			return THREAD_PRIORITY_BELOW_NORMAL;
		case ThreadPriority::Lowest:
			return THREAD_PRIORITY_LOWEST;
		case ThreadPriority::Idle:
			return THREAD_PRIORITY_IDLE;
		default:
			return THREAD_PRIORITY_NORMAL;
		}
	}
}

GMThread::GMThread()
{
	D(d);
	d->state = ThreadState::NotRunning;
	d->callback = nullptr;
}

GMThread::~GMThread()
{
	D(d);
	::CloseHandle((HANDLE)d->handle);
}

void GMThread::setPriority(ThreadPriority p)
{
	D(d);
	d->priority = p;
}

void GMThread::start()
{
	D(d);
	if (d->callback)
		d->callback->onCreateThread(this);
	d->done = false;
	d->handle = (GMThreadHandle)_beginthreadex(nullptr, 0, &threadProc, this, CREATE_SUSPENDED, nullptr);
	::SetThreadPriority((HANDLE)d->handle, toWin32ThreadPriority(d->priority));
	::ResumeThread((HANDLE)d->handle);
}

void GMThread::setCallback(IThreadCallback* callback)
{
	D(d);
	d->callback = callback;
}

void GMThread::terminateThread(GMint32 ret)
{
	D(d);
	::TerminateThread((HANDLE)d->handle, ret);
}

bool GMThread::join(GMuint32 milliseconds)
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

void GMThread::sleep(GMint32 milliseconds)
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