#include "stdafx.h"
#include "check.h"
#include "../../utilities/tools.h"

void GMAtomic::increase(GMuint* n)
{
	InterlockedIncrement(n);
}

void GMAtomic::decrease(GMuint* n)
{
	InterlockedDecrement(n);
}

GMString GMConvertion::toCurrentEnvironmentString(const GMString& string)
{
	return toWin32String(string);
}