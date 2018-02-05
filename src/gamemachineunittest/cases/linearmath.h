#ifndef __LINEARMATH_H__
#define __LINEARMATH_H__
#include "unittest_def.h"
#include <gamemachine.h>
#include "unittestcase.h"

namespace cases
{
	struct LinearMath : public UnitTestCase
	{
	public:
		virtual void addToUnitTest(UnitTest& ut) override;
	};
}

#endif