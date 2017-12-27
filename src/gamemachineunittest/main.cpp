#include <stdafx.h>
#include "cases/objectmove.h"
#include "cases/hook.h"

int main(int argc, char* argv)
{
	UnitTest unitTest;

	UnitTestCase* caseArray[] = {
		new cases::ObjectMove(),
		new cases::Hook(),
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