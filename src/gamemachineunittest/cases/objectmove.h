#ifndef __CASES_OBJECT_MOVE_H__
#define __CASES_OBJECT_MOVE_H__
#include "unittest_def.h"
#include <gamemachine.h>
#include "unittestcase.h"

namespace {
	GM_PRIVATE_OBJECT(_Base)
	{
		gm::GMint a;
		gm::GMint b;
	};

	class _Base : public gm::GMObject
	{
		GM_DECLARE_PRIVATE(_Base)
		GM_ALLOW_COPY_MOVE(_Base)

	public:
		_Base() = default;
	};

	GM_PRIVATE_OBJECT(_Derived)
	{
		gm::GMint c;
		gm::GMint d;
	};

	class _Derived : public _Base
	{
		GM_DECLARE_PRIVATE_AND_BASE(_Derived, _Base)
		GM_ALLOW_COPY_MOVE(_Derived)

	public:
		_Derived() = default;

		_Derived(int _a, int _b, int _c, int _d)
		{
			D(d);
			D_BASE(db, Base);
			db->a = _a;
			db->b = _b;
			d->c = _c;
			d->d = _d;
		}

		int _a()
		{
			D_BASE(d, Base);
			return d->a;
		}
		int _b()
		{
			D_BASE(d, Base);
			return d->b;
		}
		int _c()
		{
			D(d);
			return d->c;
		}
		int _d()
		{
			D(d);
			return d->d;
		}
	};
}

namespace cases
{
	struct ObjectMove : public UnitTestCase
	{
	public:
		ObjectMove();

	public:
		virtual void addToUnitTest(UnitTest& ut) override;

	private:
		_Derived d1_, d2_;
	};
}

#endif