#include "stdafx.h"
#include "objectmove.h"

cases::ObjectMove::ObjectMove()
	: d1_(1, 2, 3, 4)
	, d2_(11, 12, 13, 14)
{
}

void cases::ObjectMove::addToUnitTest(UnitTest& ut)
{
	d2_ = std::move(d1_);

	ut.addTestCase("使用右值构造函数，其实是交换数据", [&]() {
		return
			d2_._a() == 1 &&
			d2_._b() == 2 &&
			d2_._c() == 3 &&
			d2_._d() == 4 &&
			d1_._a() == 11 &&
			d1_._b() == 12 &&
			d1_._c() == 13 &&
			d1_._d() == 14;
	});
}