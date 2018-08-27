#ifndef __ZIP_H__
#define __ZIP_H__
#include "unittest_def.h"
#include <gamemachine.h>
#include "unittestcase.h"

namespace cases
{
	struct Base64 : public UnitTestCase
	{
	public:
		virtual void addToUnitTest(UnitTest& ut) override;
	};
}

#endif