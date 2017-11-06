#include "stdafx.h"
#include "check.h"
#include "gamemachine.h"

GMEvent::GMEvent(bool manualReset, bool initialState)
{
	D(d);
	d->handle = ::CreateEvent(NULL, manualReset, initialState ? TRUE : FALSE, _L(""));
}

GMEvent::~GMEvent()
{
	D(d);
	::CloseHandle(d->handle);
}

void GMEvent::wait(GMuint milliseconds)
{
	D(d);
	milliseconds = !milliseconds ? INFINITE : milliseconds;
	::WaitForSingleObject(d->handle, milliseconds);
}

void GMEvent::set()
{
	D(d);
	::SetEvent(d->handle);
}

void GMEvent::reset()
{
	D(d);
	::ResetEvent(d->handle);
}

GMAutoResetEvent::GMAutoResetEvent(bool initialState)
	: GMEvent(false, initialState)
{
}

GMManualResetEvent::GMManualResetEvent(bool initialState)
	: GMEvent(true, initialState)
{
}

GMfloat GMConvertion::pointToInch(GMint pt)
{
	return pt / 72.f;
}

GMfloat GMConvertion::pointToPixel(GMint pt)
{
	return pointToInch(pt) * GMScreen::dpi();
}

bool GMConvertion::hexToRGB(const GMString& hex, GMfloat rgb[3])
{
	// 将#aabbcc形式的十六进制颜色值转换成0~1之间的浮点形式
	GM_ASSERT(hex.length() == 7);
	struct __InvalidHex : public std::runtime_error {
		__InvalidHex() : std::runtime_error("Invalid hex format.") {}
	};
	auto toDecFromHex = [](char ch) -> GMint {
		switch (ch)
		{
		case '0':
			return 0;
		case '1':
			return 1;
		case '2':
			return 2;
		case '3':
			return 3;
		case '4':
			return 4;
		case '5':
			return 5;
		case '6':
			return 6;
		case '7':
			return 7;
		case '8':
			return 8;
		case '9':
			return 9;
		case 'A':
		case 'a':
			return 10;
		case 'B':
		case 'b':
			return 11;
		case 'C':
		case 'c':
			return 12;
		case 'D':
		case 'd':
			return 13;
		case 'E':
		case 'e':
			return 14;
		case 'F':
		case 'f':
			return 15;
		default:
			throw __InvalidHex();
		}
	};

	if (hex[0] != '#')
		return false;

	try
	{
		rgb[0] = ((toDecFromHex(hex[1]) << 4) + toDecFromHex(hex[2])) / 255.f;
		rgb[1] = ((toDecFromHex(hex[3]) << 4) + toDecFromHex(hex[4])) / 255.f;
		rgb[2] = ((toDecFromHex(hex[5]) << 4) + toDecFromHex(hex[6])) / 255.f;
		return true;
	}
	catch (__InvalidHex)
	{
		return false;
	}
}