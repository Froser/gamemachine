#include "stdafx.h"
#include "gmthreads.h"
#if _WINDOWS
#	include <process.h>
#endif

static GMCS cs;

GMLocker::GMLocker(GMCS& _cs)
	: cs(_cs)
{
#if _WINDOWS
	::InitializeCriticalSection(&cs);
	::EnterCriticalSection(&cs);
#else
#	error
#endif
}

GMLocker::~GMLocker()
{
#if _WINDOWS
	::LeaveCriticalSection(&cs);
	::DeleteCriticalSection(&cs);
#else
#	error
#endif
}

GMThread::GMThread()
{
	D(d);
	d->state = NotRunning;
	d->callback = nullptr;
}

unsigned int __stdcall gmthread_run(PVOID lpThreadParameter)
{
	GMThread* thread = static_cast<GMThread*>(lpThreadParameter);
	thread->d()->state = Running;
	if (thread->d()->callback)
		thread->d()->callback->beforeRun(thread);
	thread->run();
	if (thread->d()->callback)
		thread->d()->callback->afterRun(thread);
	thread->d()->state = Finished;
	thread->d()->event.set();
	return 0;
}

void GMThread::start()
{
	D(d);
#if _WINDOWS
	d->event.reset();
	d->handle = (GMThreadHandle)::_beginthreadex(NULL, NULL, gmthread_run, this, 0, 0);
	if (d->callback)
		d->callback->onCreateThread(this);
#endif
}

void GMThread::wait(GMuint milliseconds)
{
	D(d);
#if _WINDOWS
	::WaitForSingleObject(d->handle, milliseconds == 0 ? INFINITE : milliseconds);
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
	::TerminateThread(d->handle, 0);
#endif
}

GMThread::Data* GMThread::d()
{
	D(d);
	return d;
}

GMThreadId GMThread::getCurrentThreadId()
{
#if _WINDOWS
	return ::GetCurrentThreadId();
#endif
}

GMSustainedThread::GMSustainedThread()
{
	D(d);
	d->terminate = false;
	d->jobStartEvent.reset();
}

GMSustainedThread::~GMSustainedThread()
{
	D(d);
	d->jobStartEvent.set();
	terminate();
}

void GMSustainedThread::run()
{
	D(d);
	while (!d->terminate)
	{
		d->jobStartEvent.wait();

		sustainedRun();

		d->jobFinishedEvent.set();
		d->jobStartEvent.reset();
	}
}

void GMSustainedThread::wait(GMint milliseconds)
{
	D(d);
	d->jobFinishedEvent.wait(milliseconds);
}

void GMSustainedThread::trigger()
{
	D(d);
	d->jobFinishedEvent.reset();
	d->jobStartEvent.set();
}

void GMSustainedThread::stop()
{
	D(d);
	d->terminate = true;
}

GMJobPool::~GMJobPool()
{
	D(d);
	for (auto iter = d->threads.begin(); iter != d->threads.end(); iter++)
	{
		delete (*iter);
	}
}

void GMJobPool::addJob(AUTORELEASE GMThread* thread)
{
	D(d);
	d->threads.push_back(thread);
	thread->setCallback(this);
	thread->start();
}

void GMJobPool::waitJobs(GMuint milliseconds)
{
	D(d);
#if _WINDOWS
	::WaitForMultipleObjects(d->handles.size(), d->handles.data(), TRUE, milliseconds == 0 ? INFINITE : milliseconds);
#endif
}

void GMJobPool::onCreateThread(GMThread* thread)
{
	D(d);
	d->handles.push_back(thread->d()->handle);
}

void GMJobPool::afterRun(GMThread* thread)
{
}

void GMJobPool::beforeRun(GMThread* thread)
{
}
