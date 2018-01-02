#include "stdafx.h"
#include "unittestcase.h"
#include <iomanip>

void assertTrue(bool ast, const std::string& msg, std::ostream& out = std::cout)
{
	out << "Unit test: " << msg.c_str() << std::setw(15);
	if (ast)
		out << "Passed!" << std::endl;
	else
		out << "Failed!" << std::endl;
}

void UnitTest::addTestCase(const std::string& msg, TestPredicate p)
{
	m_cases.push_back({ msg, p });
}

void UnitTest::run()
{
	for (auto& c: m_cases)
	{
		bool result = c.second();
		assertTrue(result, c.first);
	}
}