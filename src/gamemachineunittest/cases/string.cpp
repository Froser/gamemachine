#include "stdafx.h"
#include <functional>
#include "string.h"

void cases::String::addToUnitTest(UnitTest& ut)
{
	ut.addTestCase("GMString赋值MultiBytes", []() {
		gm::GMString test("hello");
		return
			test[0] == 'h' &&
			test[1] == 'e' &&
			test[2] == 'l' &&
			test[3] == 'l' &&
			test[4] == 'o' &&
			test[5] == 0;
	});

	ut.addTestCase("GMString赋值WideChars", []() {
		gm::GMString test(L"gamemachine");
		return test.toStdWString() == std::wstring(L"gamemachine");
	});

	ut.addTestCase("GMString转换为std::string", []() {
		gm::GMString test(L"gamemachine");
		return test.toStdString() == std::string("gamemachine");
	});

	ut.addTestCase("GMString的append操作", []() {
		gm::GMString test(L"game");
		test += "machine";
		bool b1 = test.toStdWString() == std::wstring(L"gamemachine");
		test += L"soeasy";
		bool b2 = test.toStdWString() == std::wstring(L"gamemachinesoeasy");
		return b1 && b2;
	});

	ut.addTestCase("GMString获取单个char", []() {
		return gm::GMString("abcdefg")[3] == 'd';
	});

	ut.addTestCase("GMString比较", []() {
		gm::GMString a("gamemachine"), b(L"gamemachine");
		return a == b;
	});

	ut.addTestCase("GMString从整数构造", []() {
		return gm::GMString(5) == "5";
	});

	ut.addTestCase("GMString从浮点数构造", []() {
		return gm::GMString(5.f) == "5.000000";
	});

	ut.addTestCase("GMString replace", []() {
		gm::GMString text(L"Hello, welcome to ${project}! ${project} is very easy.");
		gm::GMString result = text.replace(L"${project}", L"gamemachine");
		return result == L"Hello, welcome to gamemachine! gamemachine is very easy.";
	});

	ut.addTestCase("GMString::stringCopy", []() {
		gm::GMwchar dest[20];
		gm::GMwchar* src = L"gamemachine";
		gm::GMString::stringCopy(dest, src);
		return gm::GMString(dest) == L"gamemachine";
	});

	ut.addTestCase("GMString::stringCat", []() {
		gm::GMwchar dest[20] = { L'g', L'a', L'm', L'e', 0 };
		gm::GMwchar* src = L"machine";
		gm::GMString::stringCat(dest, src);
		return gm::GMString(dest) == L"gamemachine";
	});

	ut.addTestCase("GMString::parseInt", []() {
		bool ok;
		gm::GMString integer("3");
		bool legalInt = gm::GMString::parseInt(integer, &ok) == 3 && ok;

		gm::GMString notAInteger("abc12abc");
		gm::GMString::parseInt(notAInteger, &ok);
		bool illegalInt = !ok;
		return legalInt && illegalInt;
	});

	ut.addTestCase("GMString::parseFloat", []() {
		bool ok;
		gm::GMString floatNumber("3.14159f");
		bool legalInt = gm::GMString::parseFloat(floatNumber, &ok) == 3.14159f && ok;

		gm::GMString notAFloat("abc3.14159fabc");
		gm::GMString::parseFloat(notAFloat, &ok);
		bool illegalFloat = !ok;
		return legalInt && illegalFloat;
	});
}
