#include "stdafx.h"
#include "gminput.h"
#include "foundation/platforms/windows/input.h"

void GMInputFactory::createInput(IInput** out)
{
#if _WINDOWS
	(*out) = new GMInput_Windows();
#else
	(*out) = nullptr;
#endif
}
