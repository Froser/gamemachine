#ifndef __GMTHREADS_H__
#define __GMTHREADS_H__
#include <gmcommon.h>
#include "utilities/utilities.h"
#include <mutex>
#include <thread>
BEGIN_NS

typedef std::thread GMThreadHandle;

enum ThreadState
{
	NotRunning,
	Running,
	Finished,
};

class GMThread;
struct IThreadCallback
{
	virtual ~IThreadCallback() {}
	virtual void onCreateThread(GMThread*) = 0;
	virtual void beforeRun(GMThread*) = 0;
	virtual void afterRun(GMThread*) = 0;
};

GM_PRIVATE_OBJECT(GMThread)
{
	IThreadCallback* callback = nullptr;
	GMThreadHandle handle;
	GMEvent event;
	ThreadState state;
	bool done = false;
};

class GMThread : public GMObject
{
	DECLARE_PRIVATE(GMThread)

public:
	GMThread();
	GMThread(GMThread&&) noexcept;

public:
	void start();
	void wait(GMuint milliseconds = 0);
	void setCallback(IThreadCallback* callback);
	void terminate();

public:
	GMThreadHandle& handle() { D(d); return d->handle; }
	GMEvent& waitEvent() { D(d); return d->event; }
	bool isDone() { D(d); return d->done; }

public:
	virtual void run() = 0;

private:
	void threadCallback();

public:
	static GMThreadHandle::id getCurrentThreadId();
	static void sleep(GMint miliseconds);
};

// GMSustainedThread
GM_PRIVATE_OBJECT(GMSustainedThread)
{
	GMEvent jobFinishedEvent;
	GMEvent jobStartEvent;
	GMEvent terminateEvent;
	bool terminate;
};

GM_PRIVATE_OBJECT(GMMutex)
{
	std::mutex mutex;
};

class GMMutex : public GMObject
{
	DECLARE_PRIVATE(GMMutex)

public:
	GMMutex() { D(d); d->mutex.lock(); }
	~GMMutex() { D(d); d->mutex.unlock(); }
};

class GMInterlock : public GMObject
{
public:
	static GMuint increment(GMuint* i) { return ::InterlockedIncrement(i); }
};

// 同步
GM_INTERFACE(IAsyncResult)
{
	virtual GMObject* state() = 0;
	virtual bool isComplete() = 0;
	virtual GMEvent& waitHandle() = 0;
};

using GMAsyncCallback = std::function<void(IAsyncResult*)>;

END_NS
#endif