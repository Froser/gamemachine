#ifndef __GMTHREADS_H__
#define __GMTHREADS_H__
#include "common.h"
#include "utilities/utilities.h"
#include <mutex>
BEGIN_NS

#if _WINDOWS
typedef HANDLE GMThreadHandle;
typedef DWORD GMThreadId;
typedef CRITICAL_SECTION GMCS;
#else
#error need implement
#endif

class GMLocker
{
public:
	GMLocker(GMCS&);
	~GMLocker();
	GMCS cs;
};

#define GMBeginLock(cs) { GMLocker __gmlocker(cs);
#define GMEndLock() }

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
	Data* d();

public:
	virtual void run() = 0;

public:
	static GMThreadId getCurrentThreadId();
};

// GMSustainedThread
GM_PRIVATE_OBJECT(GMSustainedThread)
{
	GMEvent jobFinishedEvent;
	GMEvent jobStartEvent;
	bool terminate;
};

class GMSustainedThread : public GMThread
{
	DECLARE_PRIVATE(GMSustainedThread)

public:
	GMSustainedThread();
	~GMSustainedThread();

public:
	void wait(GMint milliseconds = 0);
	void trigger();
	void stop();

public:
	virtual void run() override;

#if MULTI_THREAD
private:
#else
public:
#endif
	virtual void sustainedRun() = 0;
};

class GMSustainedThreadRunner
{
public:
	GMSustainedThreadRunner(GMSustainedThread* t) : th(t)
	{
		th->trigger();
	}

	~GMSustainedThreadRunner()
	{
		th->wait();
	}

private:
	GMSustainedThread* th;
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

#if MULTI_THREAD
#	define gmRunSustainedThread(name, thread) GMSustainedThreadRunner name(thread);
#else
#	define gmRunSustainedThread(name, thread) (thread)->sustainedRun();
#endif

GM_PRIVATE_OBJECT(SustainedThreadRunGuard)
{
	Vector<GMSustainedThread*> threads;
};

class SustainedThreadRunGuard : public GMObject
{
	DECLARE_PRIVATE(SustainedThreadRunGuard)

public:
	SustainedThreadRunGuard() = default;
	void add(GMSustainedThread* t) { GMMutex m; D(d); d->threads.push_back(t); }

#if MULTI_THREAD
	~SustainedThreadRunGuard() { GMMutex m; D(d); for (auto& thread : d->threads) { thread->wait(); } }
	void trigger(GMSustainedThread* t) { GMMutex m; D(d); for (auto& thread : d->threads) { thread->trigger(); } }
#else
	void trigger(GMSustainedThread* t) { for (auto& thread : d->threads) { thread->sustainedRun(); } }
#endif
};

// GMJobPool
GM_PRIVATE_OBJECT(GMJobPool)
{
	Vector<GMThreadHandle> handles;
	Vector<GMThread*> threads;
};

class GMJobPool : public GMObject, public IThreadCallback
{
	DECLARE_PRIVATE(GMJobPool)

public:
	GMJobPool() {}
	~GMJobPool();

public:
	void addJob(AUTORELEASE GMThread* thread);
	void waitJobs(GMuint milliseconds = 0);

public:
	virtual void onCreateThread(GMThread* thread) override;
	virtual void beforeRun(GMThread* thread) override;
	virtual void afterRun(GMThread* thread) override;
};

class GMInterlock : public GMObject
{
public:
	static GMuint increment(GMuint* i) { return ::InterlockedIncrement(i); }
};

END_NS
#endif