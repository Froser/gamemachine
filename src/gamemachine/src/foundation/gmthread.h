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
	ThreadState state;
	bool done = false;
};

//! 表示一个线程类。
/*!
  通过继承此类来定义一个线程。
*/
class GMThread : public GMObject
{
	DECLARE_PRIVATE(GMThread)

public:
	//! 类的构造函数。
	GMThread();

public:
	//! 开始一个线程。
	/*!
	  通过调用系统的创建线程函数，创建一个新线程。在新建的线程中，将会调用run()方法。
	  \sa run()
	*/
	void start();

	//! 设置线程回调。
	/*!
	  在线程进行的每一个阶段，如线程开始前、结束前，回调函数都会被调用。如果需要在线程不同阶段执行一些代码，通过此函数可以设置其回调接口。
	*/
	void setCallback(IThreadCallback* callback);

	//! 终止当前线程。
	/*!
	  尽量不要使用此方法，因为它会导致应用层面的一些资源泄漏。<br>
	  正确的行为是，应该让线程的代码走到最末尾，自然而然结束线程。
	  \param ret 线程结束返回值。
	  \sa run()
	*/
	void terminateThread(GMint ret = 0);

	//! 阻塞调用线程，等待此线程结束。
	/*!
	  当主调线程调用此方法后，主调线程将会被阻塞，等待此线程执行完毕。
	  \param miliseconds 等待此线程的结束时间。如果超过时间，线程仍然未结束，则此方法返回值为false，否则返回值为true。单位为毫秒，如果为0表示永远等待下去。
	  \return 该线程是否在规定时间内返回。
	*/
	bool join(GMuint milliseconds = 0);

	//! 获得线程ID。
	/*！
	  \return 线程ID。
	*/
	GMThreadId getThreadId();

public:
	//! 获得线程句柄。
	/*！
	  \return 线程句柄。
	*/
	GMThreadHandle& handle() { D(d); return d->handle; }

	//! 获得线程是否结束。
	/*！
	  \return 线程是否结束。
	*/
	bool isDone() { D(d); return d->done; }

public:
	//! 线程执行函数。
	/*!
	  通过重载此函数，完成用户想要完成的行为。
	*/
	virtual void run() = 0;

public:
	//! 获取线程的私有数据。
	/*!
	  私有数据包含句柄、回调对象等信息。请不要修改私有数据，除非你知道在做什么。一般情况下，也不需要调用此函数。
	*/
	GM_PRIVATE_NAME(GMThread)* _thread_private()
	{
		return data();
	}

public:
	//! 获得当前线程ID。
	/*！
	  \return 当前线程ID。
	*/
	static GMThreadId getCurrentThreadId();

	//! 将当前线程睡眠一段时间。
	/*！
	  将调用的线程睡眠一段时间，这段时间只是一个大概值，取决于操作系统如何调度线程。
	  \param milliseconds 睡眠时间，单位为毫秒。
	*/
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
	virtual void wait() = 0;
};

using GMAsyncCallback = std::function<void(IAsyncResult*)>;
END_NS
#endif
