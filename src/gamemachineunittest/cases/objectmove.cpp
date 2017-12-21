#include "stdafx.h"
#include "objectmove.h"

cases::ObjectMove::ObjectMove()
	: d1_(1, 2, 3, 4)
	, d2_(10, 11, 12, 13)
{
}

void cases::ObjectMove::addToUnitTest(UnitTest& ut)
{
	gm::gmSwap(d1_, d2_);

	ut.addTestCase("当两个GMObject进行交换时，是否第一个对象交换成功", [&]() {
		return d1_._a() == 10 &&
			d1_._b() == 11 &&
			d1_._c() == 12 &&
			d1_._d() == 13;
	});

	ut.addTestCase("当两个GMObject进行交换时，是否第二个对象交换成功", [&]() {
		return d2_._a() == 1 &&
			d2_._b() == 2 &&
			d2_._c() == 3 &&
			d2_._d() == 4;
	});
}