#include "stdafx.h"
#include "check.h"
#include "../../gamemachine.h"

namespace
{
	GMEndiannessMode getMachineEndianness()
	{
		long int i = 1;
		const char *p = (const char *)&i;
		if (p[0] == 1)
			return GMEndiannessMode::LittleEndian;
		return GMEndiannessMode::BigEndian;
	}
}


void GameMachine::runEventLoop()
{
}

void GameMachine::translateSystemEvent(GMuint uMsg, GMWParam wParam, GMLParam lParam, OUT GMSystemEvent** event)
{
}

void GameMachine::initSystemInfo()
{
	D(d);
	static bool inited = false;
	if (!inited)
	{
		d->states.systemInfo.endiannessMode = getMachineEndianness();
		inited = true;
	}
}