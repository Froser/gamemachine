#include "stdafx.h"
#include "gmmessage.h"

GMSystemEvent::GMSystemEvent(GMSystemEventType type)
{
	D(d);
	setType(type);
	d->message.objPtr = this;
}