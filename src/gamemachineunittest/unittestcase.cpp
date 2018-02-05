#include "stdafx.h"
#include "unittestcase.h"
#include <iomanip>

namespace
{
#if GM_WINDOWS
	void SetConsoleColor(WORD foreColor)
	{
		HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(handle, foreColor);
	}
#else
	void SetConsoleColor(unsigned short)
	{
		// Nothing
	}
#endif
}

void assertTrue(bool ast, const std::string& msg, std::ostream& out = std::cout)
{
	::SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	out << "Unit test: " << msg.c_str() << std::setw(15);
	if (ast)
	{
		::SetConsoleColor(FOREGROUND_GREEN);
		out << "Passed!" << std::endl;
	}
	else
	{
		::SetConsoleColor(FOREGROUND_RED);
		out << "Failed!" << std::endl;
	}
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