#ifndef __STRING_H__
#define __STRING_H__
#include "unittest_def.h"
#include <gamemachine.h>
#include "unittestcase.h"

namespace cases
{
	struct String : public UnitTestCase
	{
	public:
		virtual void addToUnitTest(UnitTest& ut) override;
	};
}

#endif