#ifndef __GMTHREADS_H__
#define __GMTHREADS_H__
#include "common.h"
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
	IThreadCallback* callback;
	GMThreadHandle handle;
	GMEvent event;
	ThreadState state;
};

class GMThread : public GMObject
{
	DECLARE_PRIVATE(GMThread)

public:
	GMThread();

public:
	void start();
	void wait(GMuint milliseconds = 0);
	void setCallback(IThreadCallback* callback);
	void terminate();

public:
	GMThreadHandle& handle() { D(d); return d->handle; }

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

END_NS
#endif