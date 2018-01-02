#ifndef __SCANNER_H__
#define __SCANNER_H__
#include "unittest_def.h"
#include <gamemachine.h>
#include "unittestcase.h"

namespace cases
{
	struct Scanner : public UnitTestCase
	{
	public:
		virtual void addToUnitTest(UnitTest& ut) override;
	};
}

#endif