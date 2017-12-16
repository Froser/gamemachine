#define GLEW_STATIC
#include <stdafx.h>
#include "cases/objectmove.h"

void static_test()
{
	{
		gm::GMObject a, b;
		// a = b; gm::GMObject不允许拷贝
	}

	{
		class Derived : gm::GMObject {};
		Derived a, b;
		// a = b; gm::GMObject派生类不允许拷贝
	}

	{
		gm::GMLua a, b;
		a = b; // 不是派生于gm::GMObject的类，是否可以拷贝取决于其实现
	}
}

int main(int argc, char* argv)
{
	UnitTest unitTest;

	UnitTestCase* caseArray[] = {
		new cases::ObjectMove()
	};

	for (auto& c : caseArray)
	{
		c->addToUnitTest(unitTest);
	}
	unitTest.run();

	for (auto& c : caseArray)
	{
		delete c;
	}

	getchar();
	return 0;
}