#include "stdafx.h"
#include "base64.h"
#include <gmutilities.h>

void cases::Base64::addToUnitTest(UnitTest& ut)
{
	ut.addTestCase("Base64", []() {
		const gm::GMwchar* str = L"welcome to gamemachine.";
		gm::GMsize_t sz;
		gm::GMBuffer bufIn;
		bufIn.buffer = (gm::GMbyte*) str;
		bufIn.size = sz = (gm::GMsize_t) wcslen(str);

		gm::GMBuffer base64 = gm::GMConvertion::toBase64(bufIn);

		gm::GMBuffer result = gm::GMConvertion::fromBase64(base64);
		return memcmp(result.buffer, str, sz) == 0;
	});
}
