#include "stdafx.h"
#include "check.h"
#include "../../utilities/tools.h"

BEGIN_NS

GMString GMConvertion::toCurrentEnvironmentString(const GMString& string)
{
	return toWin32String(string);
}

END_NS