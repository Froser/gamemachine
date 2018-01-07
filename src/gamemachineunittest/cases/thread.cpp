#include "stdafx.h"
#include "thread.h"
#include <gmthread.h>

volatile static gm::GMint g_testCode = 0;

class TestThread_Join : public gm::GMThread
{
public:
	virtual void run() override
	{
		sleep(1000);
		g_testCode = 1;
	}
};

class TestThread_Term : public gm::GMThread
{
public:
	virtual void run() override
	{
		gm::GMint i = 0;
		while (i++ < 1000)
		{
			sleep(1000);
		}
	}
};

void cases::Thread::addToUnitTest(UnitTest& ut)
{
	ut.addTestCase("GMThread start, join", [](){
		TestThread_Join t;
		t.start();
		t.join();
		return g_testCode == 1;
	});

	ut.addTestCase("GMThread sleep", []() {
		TestThread_Join t;
		t.sleep(1500);
		return true;
	});

	ut.addTestCase("GMThread terminateThread", []() {
		TestThread_Term t;
		gm::GMThread::sleep(2000);
		t.terminateThread();
		return true;
	});
}