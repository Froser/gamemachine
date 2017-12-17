#include "stdafx.h"
#include <functional>
#include "hook.h"

gm::GMString g_name1("foo1");
gm::GMString g_name2("foo2");
gm::GMString g_name3("foo3");

void cases::Hook::addToUnitTest(UnitTest& ut)
{
	// 添加一个钩子
	std::function<void(int, int, gm::GMString)> callback1 = [=](int a, int b, gm::GMString c) {
		if (a == 1 && b == 2 && c == "game")
			this->m_succeed1 = true;
	};

	// 添加第二个钩子
	std::function<void(int, int, gm::GMString)> callback2 = [=](int a, int b, gm::GMString c) {
		if (a == 3 && b == 4 && c == "machine")
			this->m_succeed2 = true;
	};

	// 添加右值引用钩子
	std::function<void(int&&, int&&, gm::GMString&)> callback3 = [=](int a, int b, gm::GMString& c) {
		if (a == 5 && b == 6 && c == "is very easy")
			this->m_succeed3 = true;
	};

	// 为已存在的钩子增加一个钩子
	std::function<void(int&&, int&&, gm::GMString&)> callback4 = [=](int a, int b, gm::GMString& c) {
		if (a == 5 && b == 6 && c == "is very easy")
			this->m_succeed4 = true;
	};

	// 安装钩子，如果钩子都调用正常，则单元测试成功
	gm::installHook(g_name1, callback1);
	gm::installHook(g_name2, callback2);
	gm::installHook(g_name3, callback3);
	gm::installHook(g_name3, callback4);

	foo1();
	foo2();
	foo3();

	ut.addTestCase("钩子函数是否被正确调用", [=](){
		return m_succeed1 && m_succeed2 && m_succeed3 && m_succeed4;
	});
}

void cases::Hook::foo1()
{
	gm::GMString game("game");
	gm::hook(g_name1, 1, 2, game);
}

void cases::Hook::foo2()
{
	gm::GMString machine("machine");
	gm::hook(g_name2, 3, 4, machine);
}

void cases::Hook::foo3()
{
	gm::GMString ez("is very easy");
	gm::hook<int&&, int&&, gm::GMString&>(g_name3, 5, 6, ez);
}