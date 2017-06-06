#ifndef __GMTHREADS_H__
#define __GMTHREADS_H__
#include "common.h"
#include "utilities/utilities.h"
BEGIN_NS

#ifdef _WINDOWS
typedef HANDLE GMThreadHandle;
typedef DWORD GMThreadId;
#else
#error need implement
#endif

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
	Data* d();

public:
	virtual void run() = 0;

public:
	static GMThreadId getCurrentThreadId();
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

END_NS
#endif