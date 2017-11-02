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

GMfloat GMUnitConvertion::pointToInch(GMint pt)
{
	return pt / 72.f;
}

GMfloat GMUnitConvertion::pointToPixel(GMint pt)
{
	return pointToInch(pt) * GMScreen::dpi();
}