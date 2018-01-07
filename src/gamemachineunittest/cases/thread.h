#ifndef __THREAD_H__
#define __THREAD_H__
#include "unittest_def.h"
#include <gamemachine.h>
#include "unittestcase.h"

namespace cases
{
	struct Thread : public UnitTestCase
	{
	public:
		virtual void addToUnitTest(UnitTest& ut) override;
	};
}

#endif