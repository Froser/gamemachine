#include "stdafx.h"
#include "thread.h"
#include <gmthread.h>
#include <gmasync.h>

#define LOOP_NUM 3
volatile static gm::GMint g_testCode = 0;

namespace gm
{
	static GMint __i;

	void beginDoSomething(GMAsyncCallback callback, OUT GMAsyncResult** ar, std::ostream& out = std::cout)
	{
		auto f = [callback, &out](GMAsyncResult* r) {
			// 这里做一个非常长时间的工作
			for (__i = 0; __i < LOOP_NUM; ++__i)
			{
				out << "工作线程正在处理：" << __i << std::endl;
				std::this_thread::sleep_for(std::chrono::seconds(3));
			}

			r->setComplete();
			callback(r);
		};

		GMAsyncResult* asyncResult = new GMAsyncResult();
		GMFuture<void> future = GMAsync::async(GMAsync::Async, f, asyncResult);
		asyncResult->setFuture(std::move(future));
		GM_ASSERT(ar);
		(*ar) = asyncResult;
	}
}

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

	ut.addTestCase("Async", []() {
		bool finished = false;
		gm::GMAsyncCallback cb = [&finished](gm::GMAsyncResult* ar) {
			std::cout << "工作线程已经完成。" << std::endl;
			finished = ar->isComplete();
		};
		gm::GMAsyncResult* ar = nullptr;
		beginDoSomething(cb, &ar);
		std::cout << "正在等待工作线程结束..." << std::endl;
		ar->wait();
		std::cout << "等待线程已经结束" << std::endl;
		gm::GM_delete(ar);
		return gm::__i == LOOP_NUM && finished;
	});
}