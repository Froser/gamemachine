#ifndef __HOOK_H__
#define __HOOK_H__
#include "unittest_def.h"
#include <gamemachine.h>
#include "unittestcase.h"

namespace {

}

namespace cases
{
	struct Hook : public UnitTestCase
	{
	public:
		virtual void addToUnitTest(UnitTest& ut) override;

	private:
		void foo1();
		void foo2();
		void foo3();

	private:
		bool m_succeed1 = false;
		bool m_succeed2 = false;
		bool m_succeed3 = false;
		bool m_succeed4 = false;
	};
}

#endif