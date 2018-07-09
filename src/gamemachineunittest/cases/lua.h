#ifndef __CASES_LUA_H__
#define __CASES_LUA_H__
#include "unittest_def.h"
#include <gamemachine.h>
#include "unittestcase.h"
#include <gmlua.h>

namespace cases
{
	struct Lua : public UnitTestCase
	{
	public:
		Lua();

	public:
		virtual void addToUnitTest(UnitTest& ut) override;

	private:
		gm::GMOwnedPtr<gm::GMObject> m_obj;
		gm::GMLua m_lua;
	};
}

#endif