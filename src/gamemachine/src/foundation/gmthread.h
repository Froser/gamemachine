#ifndef __GMTHREADS_H__
#define __GMTHREADS_H__
#include <gmcommon.h>
#include "utilities/tools.h"
#include <mutex>
#if GM_UNIX
#include <pthread.h>
#endif

BEGIN_NS

#if GM_WINDOWS
typedef void* GMThreadHandle;
typedef GMlong GMThreadId;
#elif GM_UNIX
// USE pthread
typedef pthread_t GMThreadHandle;
typedef pthread_t GMThreadId;
typedef pthread_attr_t GMThreadAttr;
#endif

enum class ThreadState
{
	NotRunning,
	Running,
	Finished,
};

enum class ThreadPriority
{
	TimeCritial,
	Highest,
	AboveNormal,
	Normal,
	BelowNormal,
	Lowest,
	Idle,
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
	GMThreadHandle handle = 0;
	ThreadState state;
	ThreadPriority priority = ThreadPriority::Normal;
#if GM_UNIX
	GMThreadAttr attr;
#endif
	bool done = false;
};

//! 表示一个线程类。
/*!
  通过继承此类来定义一个线程。
*/
class GM_EXPORT GMThread : public GMObject
{
	GM_DECLARE_PRIVATE(GMThread)

public:
	//! 类的构造函数。
	/*!
	  注意：请确保在线程运行时，此类不会被析构，否则程序将会崩溃。<br>
	  使用join()可以确保线程已经结束。
	  \sa join()
	*/
	GMThread();

	//! 类的析构函数。
	/*!
	  注意：在线程结束前，不要释放此类，否则会引起崩溃。
	*/
	~GMThread();

public:
	//! 设置一个线程优先级。
	/*!
	  优先级应该在进程开始运行前设置，否则不会有效果。
	  \param p 线程优先级。
	*/
	void setPriority(ThreadPriority p);

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
	void terminateThread(GMint32 ret = 0);

	//! 阻塞调用线程，等待此线程结束。
	/*!
	  当主调线程调用此方法后，主调线程将会被阻塞，等待此线程执行完毕。
	  \param miliseconds 等待此线程的结束时间。如果超过时间，线程仍然未结束，则此方法返回值为false，否则返回值为true。单位为毫秒，如果为0表示永远等待下去。
	  \return 该线程是否在规定时间内返回。
	*/
	bool join(GMuint32 milliseconds = 0);

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
	static void sleep(GMint32 milliseconds);
};

GM_PRIVATE_OBJECT(GMMutex)
{
#if GM_WINDOWS
	HANDLE mutex;
#endif
#if GM_UNIX
	pthread_mutex_t mutex;
#endif
};

//! 此类表示一个互斥量
class GM_EXPORT GMMutex : public GMObject
{
	GM_DECLARE_PRIVATE(GMMutex)

public:
	//! 构造一个互斥量。
	GMMutex();

	//! 释放一个互斥量。
	~GMMutex();

public:
	//! 锁定此互斥量。
	/*!
	  锁定互斥量时会进行检查。如果此互斥量已经被其他线程锁定，此方法会阻塞当前线程，直到互斥量被释放。否则，线程继续执行下去，并锁定此互斥量。lock()和unlock()必须要成对出现。
	  \sa unlock()
	*/
	void lock();

	//! 释放此互斥量。
	/*!
	  互斥量被释放后，意味着其他被此互斥量阻塞的线程其中一个可以继续执行代码。lock()和unlock()必须要成对出现。
	  \sa lock()
	*/
	void unlock();
};

struct GMMutexRelease
{
	void operator()(GMMutex* mutex) { mutex->unlock(); }
};

END_NS
#endif
