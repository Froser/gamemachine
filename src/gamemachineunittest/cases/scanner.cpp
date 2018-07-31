#include "stdafx.h"
#include <functional>
#include <vector>
#include "scanner.h"

void cases::Scanner::addToUnitTest(UnitTest& ut)
{
	ut.addTestCase("Scanner获取非空字符串", []() {
		gm::GMString input = L"game 1 2.0f machine";
		gm::GMScanner scanner(input);
		std::vector<gm::GMString> outputs;
		gm::GMString output;
		scanner.next(output);
		do {
			outputs.push_back(output);
			scanner.next(output);
		} while (!output.isEmpty());

		return outputs.size() == 4 &&
			outputs[0] == L"game" &&
			outputs[1] == L"1" &&
			outputs[2] == L"2.0f" &&
			outputs[3] == L"machine";
	});

	ut.addTestCase("Scanner字符串、数字", []() {
		gm::GMString input = L"game 1 2.0f machine";
		gm::GMScanner scanner(input);
		gm::GMString strOut, strOut2;
		gm::GMint number;
		gm::GMfloat number2;
		scanner.next(strOut);
		scanner.nextInt(number);
		scanner.nextFloat(number2);
		scanner.next(strOut2);

		return strOut == L"game" &&
			number == 1 &&
			number2 == 2.f &&
			strOut2 == L"machine";
	});

	ut.addTestCase("Scanner自定义分隔符", []() {
		gm::GMString input = L"game,1,2.0f,machine";
		gm::GMScanner scanner(input, [](gm::GMwchar ch) {
			return ch == L',';
		});
		std::vector<gm::GMString> outputs;
		gm::GMString output;
		scanner.next(output);
		do {
			outputs.push_back(output);
			scanner.next(output);
		} while (!output.isEmpty());

		return outputs.size() == 4 &&
			outputs[0] == L"game" &&
			outputs[1] == L"1" &&
			outputs[2] == L"2.0f" &&
			outputs[3] == L"machine";
	});

	ut.addTestCase("Scanner处理空数据", []() {
		gm::GMString input = L",,,";
		gm::GMScanner scanner(input, [](gm::GMwchar ch) {
			return ch == L',';
		});
		gm::GMString output;
		scanner.next(output);
		return output.isEmpty();
	});
}
