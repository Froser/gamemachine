#include "stdafx.h"
#include "thread.h"
#include <gmthread.h>

volatile static gm::GMint g_testCode = 0;

class TestThread_Join : public gm::GMThread
{
public:
	virtual void run() override
	{
		sleep(500);
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
			sleep(500);
		}
	}
};

gm::GMMutex g_testMutex;
std::vector<int> g_mutexResultList;

class TestMutex_1 : public gm::GMThread
{
public:
	virtual void run() override
	{
		sleep(1000);
		g_testMutex.lock();
		for (int i = 50; i < 100; ++i)
		{
			g_mutexResultList.push_back(i);
		}
		g_testMutex.unlock();
	}
};

class TestMutex_2 : public gm::GMThread
{
public:
	virtual void run() override
	{
		g_testMutex.lock();
		for (int i = 0; i < 50; ++i)
		{
			g_mutexResultList.push_back(i);
		}
		g_testMutex.unlock();
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
		t.start();
		gm::GMThread::sleep(700);
		return true;
	});

	ut.addTestCase("GMThread terminateThread", []() {
		TestThread_Term t;
		t.start();
		gm::GMThread::sleep(700);
		t.terminateThread();
		return true;
	});

	ut.addTestCase("GMMutex lock, unlock", []() {
		TestMutex_1 t1;
		TestMutex_2 t2;
		t1.start();
		t2.start();
		t1.join();
		t2.join();
		bool suc = true;
		for (int i = 0; i < 100; ++i)
		{
			if (i != g_mutexResultList[i])
				suc = false;
		}
		return suc;
	});
}