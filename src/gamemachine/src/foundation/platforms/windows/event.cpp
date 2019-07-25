#include "stdafx.h"
#include "check.h"
#include "../../utilities/tools.h"

BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMEvent)
{
	HANDLE handle = NULL;
};

GMEvent::GMEvent(bool manualReset, bool initialState)
{
	GM_CREATE_DATA();

	D(d);
	d->handle = ::CreateEvent(NULL, manualReset, initialState ? TRUE : FALSE, L"");
}

GMEvent::~GMEvent()
{
	D(d);
	::CloseHandle(d->handle);
}

void GMEvent::wait(GMuint32 milliseconds)
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

END_NS