#include <stdafx.h>
#include "cases/objectmove.h"
#include "cases/hook.h"
#include "cases/string.h"

int main(int argc, char* argv)
{
	UnitTest unitTest;

	UnitTestCase* caseArray[] = {
		new cases::ObjectMove(),
		new cases::Hook(),
		new cases::String(),
	};

	for (auto& c : caseArray)
	{
		c->addToUnitTest(unitTest);
	}
	unitTest.run();

	for (auto& c : caseArray)
	{
		delete c;
	}

	getchar();
	return 0;
}