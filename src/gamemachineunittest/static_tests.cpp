#include <stdafx.h>
#include <gmlua.h>

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
}