#ifndef __UNITTEST_CASE_H__
#define __UNITTEST_CASE_H__
#include "unittest_def.h"
#include <iostream>
#include <gamemachine.h>
#include <vector>

typedef std::function<bool()> TestPredicate;

class UnitTest
{
public:
	void addTestCase(const std::string&, TestPredicate&& p);
	void run();

public:
	std::vector<std::pair<std::string, TestPredicate> > m_cases;
};

class UnitTestCase
{
public:
	~UnitTestCase() {}
	virtual void addToUnitTest(UnitTest&) = 0;
};

#endif