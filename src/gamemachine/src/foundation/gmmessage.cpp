#include "stdafx.h"
#include "gmmessage.h"

GMSystemEvent::GMSystemEvent()
{
	D(d);
	d->message.objPtr = this;
}