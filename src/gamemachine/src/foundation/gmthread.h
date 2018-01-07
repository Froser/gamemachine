#ifndef __GMTHREADS_H__
#define __GMTHREADS_H__
#include <gmcommon.h>
#include "utilities/tools.h"
#include <mutex>

BEGIN_NS

typedef GMlong GMThreadHandle;
typedef GMlong GMThreadId;

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
	GMManualResetEvent event;
	ThreadState state;
	bool done = false;
};

class GMThread : public GMObject
{
	DECLARE_PRIVATE(GMThread)

public:
	GMThread();

public:
	void start();
	void setCallback(IThreadCallback* callback);
	void terminateThread(GMint ret = 0);
	bool join(GMuint milliseconds = 0);
	GMThreadId getThreadId();

public:
	GMThread::Data* threadData() { D(d); return data(); }
	GMThreadHandle& handle() { D(d); return d->handle; }
	GMEvent& waitEvent() { D(d); return d->event; }
	bool isDone() { D(d); return d->done; }

public:
	virtual void run() = 0;

public:
	GM_PRIVATE_NAME(GMThread)* _thread_private()
	{
		D(d);
		return d;
	}

public:
	static GMThreadId getCurrentThreadId();
	static void sleep(GMint milliseconds);
};

GM_PRIVATE_OBJECT(GMMutex)
{
#if GM_USE_PTHREAD
	class __MutexLocker
	{
	public:
		__MutexLocker()
		{
			pthread_mutex_init(&m_mutex, nullptr);
		}

		void lock()
		{
			pthread_mutex_lock(&m_mutex);
		}

		void unlock()
		{
			pthread_mutex_unlock(&m_mutex);
		}

	private:
		pthread_mutex_t m_mutex;
	} mutex;
#else
	std::mutex mutex;
#endif
};

class GMMutex : public GMObject
{
	DECLARE_PRIVATE(GMMutex)

public:
	GMMutex() { D(d); d->mutex.lock(); }
	~GMMutex() { D(d); d->mutex.unlock(); }
};

// 同步
GM_INTERFACE(IAsyncResult)
{
	virtual GMBuffer* state() = 0;
	virtual bool isComplete() = 0;
	virtual GMEvent& waitHandle() = 0;
};

using GMAsyncCallback = std::function<void(IAsyncResult*)>;
END_NS
#endif
